#include<pybind11/pybind11.h>
#include<pybind11/stl_bind.h>
#include<pybind11/numpy.h>

#include<comp.hpp>

using namespace ngfem;

namespace py = pybind11;

template<typename T>
py::object MoveToNumpyArray( ngstd::Array<T> &a )
{
  if(a.Size()) {
      py::capsule free_when_done(&a[0], [](void *f) {
                                 delete [] reinterpret_cast<T *>(f);
                                 });
      a.NothingToDelete();
      return py::array_t<T>(a.Size(), &a[0], free_when_done);
  }
  else
      return py::array_t<T>(0, nullptr);
}

inline IntegrationRule GetReferenceRuleNoSIMD( int dim, int order, int subdivision )
{
  IntegrationRule ir;
  int n = (order)*(subdivision+1)+1;
  const double h = 1.0/(n-1);
  if(dim==2) {
      for (auto j : Range(n))
          for (auto i : Range(n-j))
              ir.Append(IntegrationPoint(i*h, j*h, 0.0));
  }

  if(dim==3) {
      for (auto k : Range(n))
        for (auto j : Range(n-k))
            for (auto i : Range(n-j-k))
              ir.Append(IntegrationPoint(i*h, j*h, k*h));
  }

  return ir;
}

inline SIMD_IntegrationRule GetReferenceRule( int dim, int order, int subdivision )
{
  IntegrationRule ir;
  int n = (order)*(subdivision+1)+1;
  const double h = 1.0/(n-1);
  if(dim==2) {
      for (auto j : Range(n))
          for (auto i : Range(n-j))
              ir.Append(IntegrationPoint(i*h, j*h, 0.0));
  }

  if(dim==3) {
      for (auto k : Range(n))
        for (auto j : Range(n-k))
            for (auto i : Range(n-j-k))
              ir.Append(IntegrationPoint(i*h, j*h, k*h));
  }

  return SIMD_IntegrationRule(ir);
}

PYBIND11_MODULE(ngui, m) {
    m.def("GetValues", [] (shared_ptr<ngfem::CoefficientFunction> cf, shared_ptr<ngcomp::MeshAccess> ma, VorB vb, int subdivision, int order) {
            ngstd::Array<float> res;
            LocalHeap lh(10000000, "GetValues");
            int dim = ma->GetDimension();
            if(vb==BND) dim-=1;

            SIMD_IntegrationRule ir = GetReferenceRule( dim, order, subdivision );
            int nip = ir.GetNIP();
            FlatMatrix<SIMD<double>> values(ir.Size(), 1, lh);

            res.SetAllocSize(ma->GetNE(vb)*nip);

            try
              {
                for (auto el : ma->Elements(vb)) {
                  auto verts = el.Vertices();
                  HeapReset hr(lh);
                  ElementTransformation & eltrans = ma->GetTrafo (el, lh);
                  if(ma->GetDimension()==2 && vb==VOL) {
                    SIMD_MappedIntegrationRule<2,2> mir(ir, eltrans, lh);
                    cf->Evaluate(mir, values);
                  }
                  else if(ma->GetDimension()==3 && vb==BND) {
                    SIMD_MappedIntegrationRule<2,3> mir(ir, eltrans, lh);
                    cf->Evaluate(mir, values);
                  }
                  else if(ma->GetDimension()==3 && vb==VOL) {
                    SIMD_MappedIntegrationRule<3,3> mir(ir, eltrans, lh);
                    cf->Evaluate(mir, values);
                  }
                  FlatVector<double> vals(ir.GetNIP(), &values(0,0));
                  for (auto k : Range(nip))
                    res.Append(vals[k]);
                }
              }
            catch(ExceptionNOSIMD e)
              {
                res.SetSize0();
                IntegrationRule ir = GetReferenceRuleNoSIMD(dim,order,subdivision);
                FlatMatrix<double> values(ir.Size(),1,lh);
                for (auto el : ma->Elements(vb))
                  {
                    HeapReset hr(lh);
                    ElementTransformation & eltrans = ma->GetTrafo (el, lh);
                    if(ma->GetDimension()==2 && vb==VOL) {
                      MappedIntegrationRule<2,2> mir(ir, eltrans, lh);
                      cf->Evaluate(mir, values);
                    }
                    else if(ma->GetDimension()==3 && vb==BND) {
                      MappedIntegrationRule<2,3> mir(ir, eltrans, lh);
                      cf->Evaluate(mir, values);
                    }
                    else if(ma->GetDimension()==3 && vb==VOL) {
                      MappedIntegrationRule<3,3> mir(ir, eltrans, lh);
                      cf->Evaluate(mir, values);
                    }
                    for (auto k : Range(nip))
                      res.Append(values(k,0));
                  }
              }
          py::gil_scoped_acquire ac;
          return MoveToNumpyArray(res);
      },py::call_guard<py::gil_scoped_release>());

    m.def("GetMeshData", [] (shared_ptr<ngcomp::MeshAccess> ma) {
        Vector<> min(3);
        min = std::numeric_limits<double>::max();
        Vector<> max(3);
        max = std::numeric_limits<double>::lowest();

        ngstd::Array<float> vertices;
        vertices.SetAllocSize(ma->GetNP()*3);
        for ( auto vi : Range(ma->GetNP()) ) {
            auto v = ma->GetPoint<3>(vi);
            for (auto i : Range(3)) {
              vertices.Append(v[i]);
              min[i] = min2(min[i], v[i]);
              max[i] = max2(max[i], v[i]);
            }
        }

        ngstd::Array<int> elements;
        LocalHeap lh(1000000, "GetMeshData");

        int surface_elements_size = 5; // 3 vertices, 1 boundary condition index, 1 curved index
        int volume_elements_size = 6; // 4 vertices, 1 material index, 1 curved index


        VorB vb;
        int n_surface_elements;
        int n_volume_elements;
        if(ma->GetDimension()==3) {
          n_surface_elements = ma->GetNSE();
          n_volume_elements = ma->GetNE();
          vb = BND;
        }
        else {
          n_surface_elements = ma->GetNE();
          n_volume_elements = 0;
          vb = VOL;
        }

        elements.SetAllocSize(surface_elements_size*n_surface_elements + volume_elements_size*n_volume_elements);

        // 2d Elements
        int curved_index = 0;
        IntegrationRule ir;
        ir.Append(IntegrationPoint(1,0,0));
        ir.Append(IntegrationPoint(0,1,0));
        ir.Append(IntegrationPoint(0,0,0));
        ir.Append(IntegrationPoint(0.5,0.0,0.0));
        ir.Append(IntegrationPoint(0.0,0.5,0.0));
        ir.Append(IntegrationPoint(0.5,0.5,0.0));
        for (auto el : ma->Elements(vb)) {
            for (auto v : el.Vertices())
                elements.Append(v);
            elements.Append(el.GetIndex());
            elements.Append(el.is_curved ? curved_index : -1);

            if(el.is_curved) {
                HeapReset hr(lh);
                ElementTransformation & eltrans = ma->GetTrafo (el, lh);
                MappedIntegrationRule<2,3> mir(ir, eltrans, lh);
                // normals of corner vertices
                for (auto j : ngcomp::Range(3)) {
                  auto n = mir[j].GetNV();
                  for (auto i : Range(3))
                      vertices.Append(n[i]);
                }
                // mapped coordinates of edge midpoints (for P2 interpolation)
                for (auto j : ngcomp::Range(3,6)) {
                  auto p = mir[j].GetPoint();
                  for (auto i : Range(3))
                      vertices.Append(p[i]);
                }
                curved_index++;
            }
        }

        assert(elements.Size() == n_surface_elements*surface_elements_size);

        if(ma->GetDimension()==3) {
            // 3d Elements
            IntegrationRule ir;
            ir.Append(IntegrationPoint(1,0,0));
            ir.Append(IntegrationPoint(0,1,0));
            ir.Append(IntegrationPoint(0,0,1));
            ir.Append(IntegrationPoint(0,0,0));
            ir.Append(IntegrationPoint(0.5,0.0,0.0));
            ir.Append(IntegrationPoint(0.0,0.5,0.0));
            ir.Append(IntegrationPoint(0.5,0.5,0.0));
            ir.Append(IntegrationPoint(0.5,0.0,0.5));
            ir.Append(IntegrationPoint(0.0,0.5,0.5));
            ir.Append(IntegrationPoint(0.0,0.0,0.5));
            for (auto el : ma->Elements(VOL)) {
                for (auto v : el.Vertices())
                    elements.Append(v);
                elements.Append(el.GetIndex());
                elements.Append(el.is_curved ? curved_index : -1);

                if(el.is_curved) {
                    HeapReset hr(lh);
                    ElementTransformation & eltrans = ma->GetTrafo (el, lh);
                    MappedIntegrationRule<3,3> mir(ir, eltrans, lh);
                    // normals of corner vertices
                    for (auto j : ngcomp::Range(4)) {
                      auto n = mir[j].GetNV();
                      for (auto i : Range(3))
                          vertices.Append(n[i]);
                    }
                    // mapped coordinates of edge midpoints (for P2 interpolation)
                    for (auto j : ngcomp::Range(4,10)) {
                      auto p = mir[j].GetPoint();
                      for (auto i : Range(3))
                          vertices.Append(p[i]);
                    }
                    curved_index++;
                }
            }
        }

        py::gil_scoped_acquire ac;
        py::dict res;
        res["elements"] = MoveToNumpyArray(elements);
        res["n_surface_elements"] = n_surface_elements;
        res["n_volume_elements"] = n_volume_elements;
        res["volume_elements_offset"] = n_surface_elements*surface_elements_size;
        res["vertices"] = MoveToNumpyArray(vertices);
        res["min"] = min;
        res["max"] = max;
        return res;
      }, py::call_guard<py::gil_scoped_release>());
}
