#include<pybind11/pybind11.h>
#include<pybind11/stl_bind.h>
#include<pybind11/numpy.h>

#include<comp.hpp>

using namespace ngfem;

namespace py = pybind11;

template<typename T>
auto MoveToNumpyArray( ngstd::Array<T> &a )
{
  py::capsule free_when_done(&a[0], [](void *f) {
      delete [] reinterpret_cast<T *>(f);
  });
  a.NothingToDelete();
  return py::array_t<T>(a.Size(), &a[0], free_when_done);
}

PYBIND11_MODULE(ngui, m) {

    m.def("GetTetData", [] (shared_ptr<ngcomp::MeshAccess> ma) {
        LocalHeap lh(1000000, "gettetdata");
        ngstd::Array<float> coordinates;
        ngstd::Array<float> bary_coordinates;
        ngstd::Array<int> element_number;
        ngstd::Array<int> element_index;
        ngstd::Array<float> element_coordinates;
        int max_index = 0;

        size_t ntets = ma->GetNE();


        coordinates.SetAllocSize(4*ntets*3);
        bary_coordinates.SetAllocSize(4*ntets*3);
        element_coordinates.SetAllocSize(4*ntets*3);
        element_number.SetAllocSize(4*ntets);
        element_index.SetAllocSize(4*ntets);

        for (auto i : ngcomp::Range(ntets)) {
          auto ei = ElementId(VOL, i);
          auto el = ma->GetElement(ei);
          max_index = max2(el.GetIndex(),max_index);

          if(el.is_curved) {
            ///////////////
            // Handle curved elements
            auto verts = el.Vertices();
            ArrayMem<int,4> sorted_vertices{verts[0], verts[1], verts[2], verts[3]};
            ArrayMem<int,4> sorted_indices{0,1,2,3};
            ArrayMem<int,4> inverse_sorted_indices{0,1,2,3};
            BubbleSort (sorted_vertices, sorted_indices);
            Array<Vec<4,float>> ref_coords;
            Array<Vec<4,float>> ref_coords_sorted;
            int subdivision = 3;
            const int r = 1<<subdivision;
            const int s = r + 1;
            const double h = 1.0/r;
            {
                for (int i = 0; i <= r; ++i)
                    for (int j = 0; i+j <= r; ++j)
                        for (int k = 0; i+j+k <= r; ++k) {
                            Vec<4,float> p{i*h,j*h,k*h, 1.0-i*h-j*h-k*h};
                            Vec<4,float> p1;
                            ref_coords.Append(p);
                            for (auto n : Range(4))
                                p1[sorted_indices[n]] = p[n];
                            ref_coords_sorted.Append(p1);
                        }
            }
            {
                ElementTransformation & eltrans = ma->GetTrafo (ei, lh);

                Array<Vec<3>> points;
                for (auto i : Range(4))
                    inverse_sorted_indices[sorted_indices[i]] = i;
                for ( auto p : ref_coords) {
                    IntegrationPoint ip(p[0], p[1], p[2]);
                    MappedIntegrationPoint<3,3> mip(ip, eltrans);
                    points.Append(mip.GetPoint());
                }
                ArrayMem<Vec<3,float>,4> element_points;
                for(auto ip : {IntegrationPoint(1,0,0),IntegrationPoint(0,1,0),
                      IntegrationPoint(0,0,1), IntegrationPoint(0,0,0)})
                  {
                    MappedIntegrationPoint<3,3> mip(ip,eltrans);
                    element_points.Append(mip.GetPoint());
                  }

                auto EmitElement = [&] (int a, int b, int c, int d) {
                    int pi[4];
                    pi[0] = a;
                    pi[1] = b;
                    pi[2] = c;
                    pi[3] = d;


                    for (auto i : Range(4)) {
                      element_number.Append(ei.Nr());
                      element_index.Append(el.GetIndex());
                      for (auto k : Range(3)) {
                          coordinates.Append(points[pi[i]][k]);
                          bary_coordinates.Append(ref_coords_sorted[pi[i]][k]);
                          element_coordinates.Append(element_points[i][k]);
                      }
                    }
                };

                int pidx = 0;
                for (int i = 0; i <= r; ++i)
                    for (int j = 0; i+j <= r; ++j)
                        for (int k = 0; i+j+k <= r; ++k, pidx++)
                        {
                            if (i+j+k == r) continue;
                            // int pidx_curr = pidx;
                            int pidx_incr_k = pidx+1;
                            int pidx_incr_j = pidx+s-i-j;
                            int pidx_incr_i = pidx+(s-i)*(s+1-i)/2-j;

                            int pidx_incr_kj = pidx_incr_j + 1;

                            int pidx_incr_ij = pidx+(s-i)*(s+1-i)/2-j + s-(i+1)-j;
                            int pidx_incr_ki = pidx+(s-i)*(s+1-i)/2-j + 1;
                            int pidx_incr_kij = pidx+(s-i)*(s+1-i)/2-j + s-(i+1)-j + 1;

                            EmitElement(pidx,pidx_incr_k,pidx_incr_j,pidx_incr_i);
                            if (i+j+k+1 == r)
                                continue;

                            EmitElement(pidx_incr_k,pidx_incr_kj,pidx_incr_j,pidx_incr_i);
                            EmitElement(pidx_incr_k,pidx_incr_kj,pidx_incr_ki,pidx_incr_i);

                            EmitElement(pidx_incr_j,pidx_incr_i,pidx_incr_kj,pidx_incr_ij);
                            EmitElement(pidx_incr_i,pidx_incr_kj,pidx_incr_ij,pidx_incr_ki);
                            if (i+j+k+2 != r)
                                EmitElement(pidx_incr_kj,pidx_incr_ij,pidx_incr_ki,pidx_incr_kij);
                        }              
            }

          }
          else {
            auto verts = el.Vertices();

            ArrayMem<int,4> sorted_vertices{verts[0], verts[1], verts[2], verts[3]};

            BubbleSort (sorted_vertices);
            for (auto ii : Range(sorted_vertices)) {
                auto vnum = sorted_vertices[ii];
                auto v = ma->GetPoint<3>(vnum);
                for (auto k : Range(3)) {
                    coordinates.Append(v[k]);
                    bary_coordinates.Append( ii==k ? 1.0 : 0.0 );
                    element_coordinates.Append(v[k]);
                }
            }
            for (auto k : Range(4)){
              element_number.Append(i);
              element_index.Append(el.GetIndex());
            }
          }
        }


        return py::make_tuple(
            element_number.Size()/4,
            max_index,
            MoveToNumpyArray(coordinates),
            MoveToNumpyArray(bary_coordinates),
            MoveToNumpyArray(element_number),
            MoveToNumpyArray(element_index),
            MoveToNumpyArray(element_coordinates)
        );

    });
    m.def("ConvertCoefficients", [] (shared_ptr<ngcomp::GridFunction> gf) {
          auto vec = gf->GetVector().FVDouble();
          ngstd::Array<float> res;
          res.SetSize(vec.Size());
          for (auto i : Range(vec))
                res[i] = vec[i];
          py::gil_scoped_acquire ac;
          return MoveToNumpyArray(res);
      },py::call_guard<py::gil_scoped_release>());
    m.def("GetValues", [] (shared_ptr<ngfem::CoefficientFunction> cf, shared_ptr<ngcomp::MeshAccess> ma, int subdivision, int order) {
            ngstd::Array<float> res;
            LocalHeap lh(10000000, "GetValues");

            if(ma->GetDimension()==2)
            {
                auto ntrigs = ma->GetNE();
                const int npoints_trig = (order+1)*(order+2)/2;
                const int n_subtrigs = (subdivision+1)*(subdivision+1);
                int n = order*(subdivision+1)+1;
                res.SetAllocSize(ntrigs*n_subtrigs*npoints_trig);

                IntegrationRule ir;
                const double h = 1.0/(n-1);
                for (auto j : Range(n))
                    for (auto i : Range(n-j))
                        ir.Append(IntegrationPoint(i*h, j*h, 0.0));
                SIMD_IntegrationRule sir(ir);
                FlatMatrix<SIMD<double>> values(ir.Size(), 1, lh);

                for (auto i : ngcomp::Range(ntrigs)) {
                    auto ei = ElementId(VOL,i);
                    auto el = ma->GetElement(ElementId( VOL, i));
                    auto verts = el.Vertices();
                    HeapReset hr(lh);
                    ElementTransformation & eltrans = ma->GetTrafo (ei, lh);
                    SIMD_MappedIntegrationRule<2,2> mir(ir, eltrans, lh);
                    cf->Evaluate(mir, values);
                    int k = 0;
                    for (auto v : FlatVector<double>(ir.Size(), &values(0,0))) {
                        if (k++<ir.Size())
                          res.Append(v);
                    }
                }
            }
          py::gil_scoped_acquire ac;
          return MoveToNumpyArray(res);
      },py::call_guard<py::gil_scoped_release>());
    m.def("GetFaceData", [] (shared_ptr<ngcomp::MeshAccess> ma) {
        ngstd::Array<float> coordinates;
        ngstd::Array<float> bary_coordinates;
        ngstd::Array<int> element_number;
        ngstd::Array<int> element_index;
        size_t ntrigs;
        int max_index=0;

        Vector<> min(3);
        min = std::numeric_limits<double>::max();
        Vector<> max(3);
        max = std::numeric_limits<double>::lowest();

        auto addVertices = [&] (auto verts) {
//             ArrayMem<int,3> sorted_vertices{0,1,2};
//             ArrayMem<int,3> unsorted_vertices{verts[0], verts[1], verts[2]};

//             BubbleSort (unsorted_vertices, sorted_vertices);
            for (auto j : ngcomp::Range(3)) {
                auto v = ma->GetPoint<3>(verts[j]);
                for (auto k : Range(3)) {
                  coordinates.Append(v[k]);
//                   bary_coordinates.Append( unsorted_vertices[k]==verts[j] ? 1.0 : 0.0 );
                  bary_coordinates.Append( k==j ? 1.0 : 0.0 );

                  min[k] = min2(min[k], v[k]);
                  max[k] = max2(max[k], v[k]);
                }
            }
        };

        if(ma->GetDimension()==2)
        {
            ntrigs = ma->GetNE();
            element_number.SetSize(3*ntrigs);
            element_index.SetSize(3*ntrigs);
            for (auto i : ngcomp::Range(ntrigs)) {
                auto el = ma->GetElement(ElementId( VOL, i));
                addVertices(el.Vertices());
                for (auto k : Range(3)) {
                  element_number[3*i+k] = i;
                  element_index[3*i+k] = el.GetIndex();
                  max_index = max2(max_index, el.GetIndex());
                }
            }
        }
        else if(ma->GetDimension()==3)
        {
            ntrigs = ma->GetNSE();
            element_number.SetSize(3*ntrigs);
            element_index.SetSize(3*ntrigs);

            for (auto sel : ma->Elements(BND)) {
                auto sel_vertices = sel.Vertices();

                ArrayMem<int,10> elnums;
                ma->GetFacetElements (sel.Facets()[0], elnums);
                auto el = ma->GetElement(ElementId(VOL, elnums[0]));

                auto el_vertices = el.Vertices();
                ArrayMem<int,4> sorted_vertices{0,1,2,3};
                ArrayMem<int,4> unsorted_vertices{el_vertices[0], el_vertices[1], el_vertices[2], el_vertices[3]};
                BubbleSort (unsorted_vertices, sorted_vertices);

                for (auto j : ngcomp::Range(3)) {
                  auto v = ma->GetPoint<3>(sel_vertices[j]);
                  for (auto k : Range(3)) {
                    coordinates.Append(v[k]);
                    bary_coordinates.Append( unsorted_vertices[k]==sel_vertices[j] ? 1.0 : 0.0 );
                    min[k] = min2(min[k], v[k]);
                    max[k] = max2(max[k], v[k]);
                  }
                }
                for (auto k : Range(3)) {
                    element_number[3*sel.Nr()+k] = elnums[0];
                    element_index[3*sel.Nr()+k] = sel.GetIndex();
                    max_index = max2(max_index, sel.GetIndex());
                }
            }
        }
        else
            throw runtime_error("Unsupported mesh dimension: "+ToString(ma->GetDimension()));
        py::gil_scoped_acquire ac;
        return py::make_tuple(
            ntrigs,
            MoveToNumpyArray(coordinates),
            MoveToNumpyArray(bary_coordinates),
            MoveToNumpyArray(element_number),
            MoveToNumpyArray(element_index),
            max_index,
            min, max
        );
      },py::call_guard<py::gil_scoped_release>());

}
