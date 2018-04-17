#include<pybind11/pybind11.h>
#include<pybind11/stl_bind.h>
#include<pybind11/numpy.h>
#include <locale.h>

#include<comp.hpp>
#include<meshing.hpp>
#include<csg.hpp>
#include <stlgeom.hpp>
#include<type_traits>

using namespace ngfem;
using std::is_same;

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

inline IntegrationRule GetReferenceRule( int dim, int order, int subdivision )
{
  IntegrationRule ir;
  int n = (order)*(subdivision+1)+1;
  const double h = 1.0/(n-1);
  if(dim==1) {
      for (auto i : Range(n)) {
          ir.Append(IntegrationPoint(1.0-i*h, 0, 0.0));
      }
  }
  if(dim==2) {
      for (auto j : Range(n))
          for (auto i : Range(n-j))
              ir.Append(IntegrationPoint(i*h, j*h, 0.0));
  }

  if(dim==3) {
      for (auto k : Range(n))
        for (auto j : Range(n-k))
            for (auto i : Range(n-j-k))
              ir.Append(IntegrationPoint(1.0-i*h-j*h-k*h, i*h, j*h));
  }

  return ir;
}

BaseMappedIntegrationRule *GetMappedIR (IntegrationRule & ir, int dim, VorB vb, ElementTransformation & eltrans, LocalHeap &lh ) {
    BaseMappedIntegrationRule * pmir;
    if(dim==1) {
        void *p = lh.Alloc(sizeof(MappedIntegrationRule<1,1>));
        return new (p) MappedIntegrationRule<1,1> (ir, eltrans, lh);
    }
    else if(dim==2 && vb==VOL) {
        void *p = lh.Alloc(sizeof(MappedIntegrationRule<2,2>));
        return new (p) MappedIntegrationRule<2,2> (ir, eltrans, lh);
    }
    else if(dim==3 && vb==BND) {
        void *p = lh.Alloc(sizeof(MappedIntegrationRule<2,3>));
        return new (p) MappedIntegrationRule<2,3> (ir, eltrans, lh);
    }
    else if(dim==3 && vb==VOL) {
        void *p = lh.Alloc(sizeof(MappedIntegrationRule<3,3>));
        return new (p) MappedIntegrationRule<3,3> (ir, eltrans, lh);
    }
    throw Exception("GetMappedIR: unknown dimension/VorB combination");
}
SIMD_BaseMappedIntegrationRule *SIMD_GetMappedIR (SIMD_IntegrationRule & ir, int dim, VorB vb, ElementTransformation & eltrans, LocalHeap &lh ) {
    SIMD_BaseMappedIntegrationRule * pmir;
    if(dim==1) {
        void *p = lh.Alloc(sizeof(SIMD_MappedIntegrationRule<1,1>));
        return new (p) SIMD_MappedIntegrationRule<1,1> (ir, eltrans, lh);
    }
    else if(dim==2 && vb==VOL) {
        void *p = lh.Alloc(sizeof(SIMD_MappedIntegrationRule<2,2>));
        return new (p) SIMD_MappedIntegrationRule<2,2> (ir, eltrans, lh);
    }
    else if(dim==3 && vb==BND) {
        void *p = lh.Alloc(sizeof(SIMD_MappedIntegrationRule<2,3>));
        return new (p) SIMD_MappedIntegrationRule<2,3> (ir, eltrans, lh);
    }
    else if(dim==3 && vb==VOL) {
        void *p = lh.Alloc(sizeof(SIMD_MappedIntegrationRule<3,3>));
        return new (p) SIMD_MappedIntegrationRule<3,3> (ir, eltrans, lh);
    }
    throw Exception("SIMD_GetMappedIR: unknown dimension/VorB combination");
}

template <typename TSCAL>
void ExtractRealImag(const TSCAL val, int i, float &real, float &imag);

template<> void ExtractRealImag(const SIMD<Complex> val, int i, float &real, float &imag) {
    real = val.real()[i];
    imag = val.imag()[i];
}

template<> void ExtractRealImag(const Complex val, int i, float &real, float &imag) {
    real = val.real();
    imag = val.imag();
}

template<> void ExtractRealImag(const SIMD<double> val, int i, float &real, float &imag) { real = val[i]; }
template<> void ExtractRealImag(const double val, int i, float &real, float &imag) { real = val; }

template<typename TSCAL, typename TMIR>
void GetValues( const CoefficientFunction &cf, LocalHeap &lh, const TMIR &mir, FlatArray<float> values_real, FlatArray<float> values_imag , FlatArray<float> min, FlatArray<float> max) {
    static_assert( is_same<TSCAL, SIMD<double>>::value || is_same<TSCAL, SIMD<Complex>>::value || is_same<TSCAL, double>::value || is_same<TSCAL, Complex>::value, "Unsupported type in GetValues");

    HeapReset hr(lh);

    auto ncomps = cf.Dimension();
    int nip = mir.IR().GetNIP();
    auto getIndex = [=] ( int p, int comp ) { return p*ncomps + comp; };
    bool is_complex = is_same<TSCAL, SIMD<Complex>>::value || is_same<TSCAL, Complex>::value;

    if(is_same<TSCAL, SIMD<double>>::value || is_same<TSCAL, SIMD<Complex>>::value)
    {
        FlatMatrix<TSCAL> values(ncomps, mir.Size(), lh);
        cf.Evaluate(mir, values);

        constexpr int n = SIMD<double>::Size();
        for (auto k : Range(nip)) {
            for (auto i : Range(ncomps)) {
                float vreal, vimag;
                ExtractRealImag( values(i, k/n), k%n, vreal, vimag );
                auto index = getIndex(k,i);
                values_real[index] = vreal;
                if(is_complex)
                  values_imag[index] = vimag;
                min[i] = min2(min[i], vreal);
                max[i] = max2(max[i], vreal);
            }
        }
    }
    else
    {
        FlatMatrix<TSCAL> values(mir.Size(), ncomps, lh);
        cf.Evaluate(mir, values);
        for (auto k : Range(nip)) {
            for (auto i : Range(ncomps)) {
                float vreal, vimag;
                ExtractRealImag( values(i, k), 0, vreal, vimag );
                auto index = getIndex(k,i);
                values_real[index] = vreal;
                if(is_complex)
                  values_imag[index] = vimag;
                min[i] = min2(min[i], vreal);
                max[i] = max2(max[i], vreal);
            }
        }
    }

}

PYBIND11_MODULE(ngui, m) {
  m.def("SetLocale", []()
        {
          setlocale(LC_NUMERIC,"C");
        });
  m.def("GetValues", [] (shared_ptr<ngfem::CoefficientFunction> cf, shared_ptr<ngcomp::MeshAccess> ma, VorB vb, int subdivision, int order) {
            LocalHeap lh(10000000, "GetValues");
            int dim = ma->GetDimension();
            if(vb==BND) dim-=1;

            int ncomps = cf->Dimension();
            Array<float> min(ncomps);
            Array<float> max(ncomps);
            min = std::numeric_limits<float>::max();
            max = std::numeric_limits<float>::min();

            IntegrationRule ir = GetReferenceRule( dim, order, subdivision );
            SIMD_IntegrationRule simd_ir(ir);
            int nip = ir.GetNIP();

            int values_per_element = nip*ncomps;

            Array<float> res_real;
            Array<float> res_imag;

            res_real.SetSize(ma->GetNE(vb)*values_per_element); // two entries for global min/max
            if(cf->IsComplex())
                res_imag.SetSize(ma->GetNE(vb)*values_per_element); // two entries for global min/max

            try
              {
                for (auto el : ma->Elements(vb)) {
                  HeapReset hr(lh);
                  ElementTransformation & eltrans = ma->GetTrafo (el, lh);
                  SIMD_BaseMappedIntegrationRule * pmir = SIMD_GetMappedIR( simd_ir, ma->GetDimension(), vb, eltrans, lh );
                  size_t first = el.Nr()*values_per_element;
                  size_t next = (el.Nr()+1)*values_per_element;
                  if(cf->IsComplex())
                    GetValues<SIMD<Complex>>( *cf, lh, *pmir, res_real.Range(first,next), res_imag.Range(first,next), min, max);
                  else
                    GetValues<SIMD<double>>( *cf, lh, *pmir, res_real.Range(first,next), res_imag, min, max);
                }
              }
            catch(ExceptionNOSIMD e)
              {
                for (auto el : ma->Elements(vb)) {
                  HeapReset hr(lh);
                  ElementTransformation & eltrans = ma->GetTrafo (el, lh);
                  BaseMappedIntegrationRule * pmir = GetMappedIR( ir, ma->GetDimension(), vb, eltrans, lh );
                  size_t first = el.Nr()*values_per_element;
                  size_t next = (el.Nr()+1)*values_per_element;
                  if(cf->IsComplex())
                    GetValues<Complex>( *cf, lh, *pmir, res_real.Range(first,next), res_imag.Range(first,next), min, max);
                  else
                    GetValues<double>( *cf, lh, *pmir, res_real.Range(first,next), res_imag, min, max);
                }
              }
          py::gil_scoped_acquire ac;
          py::dict res;
          res["real"] = MoveToNumpyArray(res_real);
          if(cf->IsComplex())
            res["imag"] = MoveToNumpyArray(res_imag);
          res["min"] = MoveToNumpyArray(min);
          res["max"] = MoveToNumpyArray(max);
          return res;
      },py::call_guard<py::gil_scoped_release>());

    m.def("GetMeshData", [] (shared_ptr<ngcomp::MeshAccess> ma) {
        Vector<> min(3);
        min = std::numeric_limits<double>::max();
        Vector<> max(3);
        max = std::numeric_limits<double>::lowest();

        ngstd::Array<float> vertices;
        vertices.SetAllocSize(ma->GetNV()*3);
        for ( auto vi : Range(ma->GetNV()) ) {
            auto v = ma->GetPoint<3>(vi);
            for (auto i : Range(3)) {
              vertices.Append(v[i]);
              min[i] = min2(min[i], v[i]);
              max[i] = max2(max[i], v[i]);
            }
        }

        ngstd::Array<int> elements;
        LocalHeap lh(1000000, "GetMeshData");

        int edge_elements_size = 4; // 2 vertices, 1 boundary condition index, 1 curved index
        int surface_elements_size = 5; // 3 vertices, 1 boundary condition index, 1 curved index
        int volume_elements_size = 6; // 4 vertices, 1 material index, 1 curved index

        int n_edge_elements = 0;
        int n_surface_elements = 0;
        int n_volume_elements = 0;

        int meshdim = ma->GetDimension();
        switch (meshdim) {
          case 3:
            n_surface_elements = ma->GetNSE();
            n_volume_elements = ma->GetNE();
            n_edge_elements = ma->GetNE(BBND);
            break;
          case 2:
            n_surface_elements = ma->GetNE();
            n_edge_elements = ma->GetNE(BND);
            break;
          case 1:
            n_edge_elements = ma->GetNE();
            break;
        }

        size_t elsize = edge_elements_size * n_edge_elements +
                        surface_elements_size*n_surface_elements +
                        volume_elements_size*n_volume_elements;
        elements.SetAllocSize(2*elsize);

        // additional information for curved (i.e. real curved or non-simplex elements like quads)
        // first  entry: number of vertices (also determines element type)
        // second entry: offset at vertices array for additional float data (normals, edge midpoint coords etc.)
        // other entries: additional vertex indices (4th vertex for quads, 5th and other for prism, pyramid, hex)
        ngstd::Array<int> curve_info;

        if(ma->GetDimension()>=1) {
            // 1d Elements
            IntegrationRule ir;
            ir.Append(IntegrationPoint(0,0,0));
            ir.Append(IntegrationPoint(1,0,0));
            ir.Append(IntegrationPoint(0.5,0.0,0.0));
            VorB vb = ma->GetDimension() == 1 ? VOL : (ma->GetDimension() == 2 ? BND : BBND);
            for (auto el : ma->Elements(vb)) {
                auto verts = el.Vertices();
                for (auto i : Range(2))
                    elements.Append(verts[i]);
                elements.Append(el.GetIndex());
                elements.Append(el.is_curved ? elsize+curve_info.Size() : -1);

                if(el.is_curved) {
                    // TODO: curved 1d elements are untested
                    curve_info.Append(verts.Size());
                    curve_info.Append(vertices.Size()/3);
                    for (auto i : Range(2UL,verts.Size()))
                        curve_info.Append(verts[i]);

                    HeapReset hr(lh);
                    ElementTransformation & eltrans = ma->GetTrafo (el, lh);
                    MappedIntegrationRule<1,3> mir(ir, eltrans, lh);
                    // normals of corner vertices
                    for (auto j : ngcomp::Range(2)) {
                        auto n = mir[j].GetNV();
                        for (auto i : Range(3))
                            vertices.Append(n[i]);
                    }
                    // mapped coordinates of edge midpoints (for P2 interpolation)
                    for (auto j : ngcomp::Range(2,3)) {
                        auto p = mir[j].GetPoint();
                        for (auto i : Range(3))
                            vertices.Append(p[i]);
                    }
                }
            }

            assert(elements.Size() == n_edge_elements*edge_elements_size);
        }
        if(ma->GetDimension()>=2) {
            // 2d Elements
            IntegrationRule ir_trig;
            ir_trig.Append(IntegrationPoint(1,0,0));
            ir_trig.Append(IntegrationPoint(0,1,0));
            ir_trig.Append(IntegrationPoint(0,0,0));
            ir_trig.Append(IntegrationPoint(0.5,0.0,0.0));
            ir_trig.Append(IntegrationPoint(0.0,0.5,0.0));
            ir_trig.Append(IntegrationPoint(0.5,0.5,0.0));

            IntegrationRule ir_quad;
            ir_quad.Append(IntegrationPoint(0,0,0));
            ir_quad.Append(IntegrationPoint(1,0,0));
            ir_quad.Append(IntegrationPoint(1,1,0));
            ir_quad.Append(IntegrationPoint(0,1,0));
            ir_quad.Append(IntegrationPoint(0.5,0.0,0.0));
            ir_quad.Append(IntegrationPoint(0.0,0.5,0.0));
            ir_quad.Append(IntegrationPoint(0.5,0.5,0.0));
            ir_quad.Append(IntegrationPoint(1.0,0.5,0.0));
            ir_quad.Append(IntegrationPoint(0.5,1.0,0.0));
            // Todo: midpoints for p2 interpolation of coordinates

            VorB vb = ma->GetDimension() == 2 ? VOL : BND;
            for (auto el : ma->Elements(vb)) {
                auto verts = el.Vertices();
                auto nverts = verts.Size();
                for (auto i : Range(3))
                    elements.Append(verts[i]);
                elements.Append(el.GetIndex());
                elements.Append(el.is_curved ? elsize+curve_info.Size() : -1);

                if(el.is_curved) {
                    curve_info.Append(nverts);
                    curve_info.Append(vertices.Size()/3);
                    for (auto i : Range(3UL,nverts))
                        curve_info.Append(verts[i]);
                    HeapReset hr(lh);
                    ElementTransformation & eltrans = ma->GetTrafo (el, lh);
                    auto & ir = nverts == 3 ? ir_trig : ir_quad;
                    MappedIntegrationRule<2,3> mir(ir, eltrans, lh);
                    // normals of corner vertices
                    for (auto j : ngcomp::Range(nverts)) {
                        auto n = mir[j].GetNV();
                        for (auto i : Range(3))
                            vertices.Append(n[i]);
                    }
                    // mapped coordinates of edge midpoints (for P2 interpolation)
                    for (auto j : ngcomp::Range(3UL,ir.Size())) {
                        auto p = mir[j].GetPoint();
                        for (auto i : Range(3))
                            vertices.Append(p[i]);
                    }
                }
            }

            assert(elements.Size() == n_surface_elements*surface_elements_size + n_edge_elements*edge_elements_size);
        }

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
                auto verts = el.Vertices();
                for (auto i : Range(4))
                    elements.Append(verts[i]);
                elements.Append(el.GetIndex());
                elements.Append(el.is_curved ? elsize+curve_info.Size() : -1);

                if(el.is_curved) {
                    curve_info.Append(verts.Size());
                    curve_info.Append(vertices.Size()/3);
                    for (auto i : Range(4UL,verts.Size()))
                        curve_info.Append(verts[i]);

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
                }
            }
        }


        elements.SetAllocSize(elements.Size()+curve_info.Size());
        for (auto i : curve_info)
            elements.Append(i);


        py::gil_scoped_acquire ac;
        py::dict res;
        res["elements"] = MoveToNumpyArray(elements);
        res["n_edge_elements"] = n_edge_elements;
        res["n_surface_elements"] = n_surface_elements;
        res["n_volume_elements"] = n_volume_elements;
        res["volume_elements_offset"] = n_edge_elements * edge_elements_size +
          n_surface_elements*surface_elements_size;
        res["surface_elements_offset"] = n_edge_elements * edge_elements_size;
        res["vertices"] = MoveToNumpyArray(vertices);
        res["min"] = min;
        res["max"] = max;
        return res;
      }, py::call_guard<py::gil_scoped_release>());

    m.def("GetGeoData", [] (shared_ptr<netgen::NetgenGeometry> geo) -> py::dict
          {
            Array<float> vertices;
            Array<int> trigs;
            Array<float> normals;
            Array<float> min = {std::numeric_limits<float>::max(),
                                     std::numeric_limits<float>::max(),
                                     std::numeric_limits<float>::max()};
            Array<float> max = {std::numeric_limits<float>::min(),
                                     std::numeric_limits<float>::min(),
                                     std::numeric_limits<float>::min()};
            Array<string> surfnames;

            auto csg_geo = dynamic_pointer_cast<netgen::CSGeometry>(geo);
            auto stl_geo = dynamic_pointer_cast<netgen::STLGeometry>(geo);

            // CSGeometries
            if(csg_geo)
              {
                for (auto i : Range(csg_geo->GetNSurf()))
                  {
                    auto surf = csg_geo->GetSurface(i);
                    surfnames.Append(surf->GetBCName());
                  }
                csg_geo->FindIdenticSurfaces(1e-6);
                csg_geo->CalcTriangleApproximation(0.01,100);
                auto nto = csg_geo->GetNTopLevelObjects();
                size_t np = 0;
                size_t ntrig = 0;
                for (auto i : Range(nto)){
                  np += csg_geo->GetTriApprox(i)->GetNP();
                  ntrig += csg_geo->GetTriApprox(i)->GetNT();
                }
                vertices.SetAllocSize(np*3);
                trigs.SetAllocSize(ntrig*4);
                normals.SetAllocSize(np*3);
                for (auto i : Range(nto))
                  {
                    auto triapprox = csg_geo->GetTriApprox(i);
                    for (auto j : Range(triapprox->GetNP()))
                      for(auto k : Range(3)) {
                        float val = triapprox->GetPoint(j)[k];
                        vertices.Append(val);
                        min[k] = min2(min[k], val);
                        max[k] = max2(max[k],val);
                        normals.Append(triapprox->GetNormal(j)[k]);
                      }
                    for (auto j : Range(triapprox->GetNT()))
                      {
                        for(auto k : Range(3))
                            trigs.Append(triapprox->GetTriangle(j)[k]);
                        trigs.Append(triapprox->GetTriangle(j).SurfaceIndex());
                      }
                  }
              }
                // STL Geometries
            else if(stl_geo)
              {
                surfnames.Append("stl");
                vertices.SetAllocSize(stl_geo->GetNT()*3*3);
                trigs.SetAllocSize(stl_geo->GetNT()*4);
                normals.SetAllocSize(stl_geo->GetNT()*3*3);
                size_t ii = 0;
                for(auto i : Range(stl_geo->GetNT()))
                  {
                    auto& trig = stl_geo->GetTriangle(i+1);
                    for(auto k : Range(3))
                      {
                        trigs.Append(ii++);
                        auto& pnt = stl_geo->GetPoint(trig[k]);
                        for (auto l : Range(3))
                          {
                            float val = pnt[l];
                            vertices.Append(val);
                            min[l] = min2(min[l], val);
                            max[l] = max2(max[l], val);
                            normals.Append(trig.Normal()[l]);
                          }
                      }
                    trigs.Append(0);
                  }
              }
            else
              throw Exception("Couldn't create geometry information!");

            py::gil_scoped_acquire ac;
            py::dict res;
            py::list snames;
            for(auto name : surfnames)
              snames.append(py::cast(name));
            res["vertices"] = MoveToNumpyArray(vertices);
            res["triangles"] = MoveToNumpyArray(trigs);
            res["normals"] = MoveToNumpyArray(normals);
            res["surfnames"] = snames;
            res["min"] = MoveToNumpyArray(min);
            res["max"] = MoveToNumpyArray(max);
            return res;
          }, py::call_guard<py::gil_scoped_release>());
}
