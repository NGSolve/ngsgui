#include<pybind11/pybind11.h>
#include<pybind11/stl_bind.h>
#include<pybind11/numpy.h>
#include <locale.h>

#include <comp.hpp>
#include <meshing.hpp>
#include <csg.hpp>
#include <geometry2d.hpp>
#include <occgeom.hpp>
#include <stlgeom.hpp>
#include <type_traits>
#include <atomic>

using namespace ngcomp;
using std::is_same;

namespace py = pybind11;

VorB getVB(int codim) {
    switch(codim) {
      case 0: return VOL;
      case 1: return BND;
      case 2: return BBND;
      case 3: return BBBND;
      default: throw Exception("invalid codim");
    }
}

struct ElementInformation {
    ElementInformation() = default;
    ElementInformation( size_t size_, ELEMENT_TYPE type_, bool curved_=false)
      : size(size_), type(type_), curved(curved_), nelements(0) {
          switch(type) {
            case ET_POINT: nverts=1; break;
            case ET_SEGM: nverts=2; break;
            case ET_TRIG: nverts=3; break;
            case ET_QUAD: nverts=4; break;
            case ET_TET: nverts=4; break;
            case ET_HEX: nverts=8; break;
            case ET_PRISM: nverts=6; break;
            case ET_PYRAMID: nverts=5; break;
            default: throw Exception("ElementInformation(): unknown element type " + ToString(type));
          }
          switch(type) {
            case ET_POINT:
              dim=0; break;
            case ET_SEGM:
              dim=1; break;
            case ET_TRIG:
            case ET_QUAD:
              dim=2; break;
            case ET_TET:
            case ET_HEX:
            case ET_PRISM:
            case ET_PYRAMID:
              dim=3; break;
            default: throw Exception("ElementInformation(): unknown element type " + ToString(type));
          }
      }
    Array<int> data; // the data that will go into the gpu texture buffer
    size_t size;   // integer entries for each element (vertices, curved_index, number, material/boundary index)
    int nverts;
    int dim;
    ELEMENT_TYPE type;
    bool curved;
    int nelements;
};

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

IntegrationRule& GetP2Rule( ELEMENT_TYPE et ) {
    static IntegrationRule ir_segm;
    static IntegrationRule ir_trig;
    static IntegrationRule ir_quad;
    static IntegrationRule ir_tet;
    static IntegrationRule ir_prism;
    static IntegrationRule ir_pyramid;
    static IntegrationRule ir_hex;

    // first call of this function, initialize rules
    if(ir_segm.Size() == 0) {
        ir_segm.Append(IntegrationPoint(0.0,0,0));
        ir_segm.Append(IntegrationPoint(1.0,0,0));
        ir_segm.Append(IntegrationPoint(0.5,0,0));

        // for 2d elements we need to get the normal vectors at the corner vertices plus mapped coordinates of edge midpoints
        ir_trig.Append(IntegrationPoint(1,0,0));
        ir_trig.Append(IntegrationPoint(0,1,0));
        ir_trig.Append(IntegrationPoint(0,0,0));
        ir_trig.Append(IntegrationPoint(0.5,0.0,0.0));
        ir_trig.Append(IntegrationPoint(0.0,0.5,0.0));
        ir_trig.Append(IntegrationPoint(0.5,0.5,0.0));

        ir_quad.Append(IntegrationPoint(0,0,0));
        ir_quad.Append(IntegrationPoint(1,0,0));
        ir_quad.Append(IntegrationPoint(1,1,0));
        ir_quad.Append(IntegrationPoint(0,1,0));
        ir_quad.Append(IntegrationPoint(0.5,0.0,0.0));
        ir_quad.Append(IntegrationPoint(0.0,0.5,0.0));
        ir_quad.Append(IntegrationPoint(0.5,0.5,0.0));
        ir_quad.Append(IntegrationPoint(1.0,0.5,0.0));
        ir_quad.Append(IntegrationPoint(0.5,1.0,0.0));

        // 3d elements have no normal vectors, so only evaluate at edge midpoints
        ir_tet.Append(IntegrationPoint(0.5,0.0,0.0));
        ir_tet.Append(IntegrationPoint(0.0,0.5,0.0));
        ir_tet.Append(IntegrationPoint(0.5,0.5,0.0));
        ir_tet.Append(IntegrationPoint(0.5,0.0,0.5));
        ir_tet.Append(IntegrationPoint(0.0,0.5,0.5));
        ir_tet.Append(IntegrationPoint(0.0,0.0,0.5));

        // PRISM
        for (auto & ip : ir_trig.Range(3,6))
            ir_prism.Append(ip);
        for (auto & ip : ir_trig.Range(0,3))
            ir_prism.Append(IntegrationPoint(ip(0), ip(1), 0.5));
        for (auto & ip : ir_trig.Range(3,6))
            ir_prism.Append(IntegrationPoint(ip(0), ip(1), 1.0));

        // PYRAMID
        for (auto & ip : ir_quad.Range(4,9))
            ir_pyramid.Append(ip);
        for (auto & ip : ir_quad.Range(0,4))
            ir_pyramid.Append(IntegrationPoint(ip(0), ip(1), 0.5));

        // HEX
        for (auto & ip : ir_quad.Range(4,9))
            ir_hex.Append(ip);
        for (auto x : {0.0, 0.5, 1.0})
          for (auto y : {0.0, 0.5, 1.0})
            ir_hex.Append(IntegrationPoint(x,y,0.5));
        for (auto & ip : ir_quad.Range(4,9))
            ir_hex.Append(IntegrationPoint(ip(0), ip(1), 1.0));
    }

    switch (et) {
      case ET_SEGM: return ir_segm;
      case ET_TRIG: return ir_trig;
      case ET_QUAD: return ir_quad;
      case ET_TET: return ir_tet;
      case ET_PYRAMID: return ir_pyramid;
      case ET_PRISM: return ir_prism;
      case ET_HEX: return ir_hex;
    }
    throw Exception("GetP2Rule(): unknown element type");
}

inline IntegrationRule GetReferenceRule( ELEMENT_TYPE et, int order, int subdivision )
{
  IntegrationRule ir;
  int n = (order)*(subdivision+1)+1;
  const double h = 1.0/(n-1);
  if(et == ET_SEGM) {
      for (auto i : Range(n)) {
          ir.Append(IntegrationPoint(1.0-i*h, 0, 0.0));
      }
  }
  else if(et==ET_TRIG || et==ET_QUAD) {
      for (auto j : Range(n))
          for (auto i : Range(n)) {
              if(et==ET_QUAD || i+j<n) // skip i+j>=n for trigs
                  ir.Append(IntegrationPoint(i*h, j*h, 0.0));
          }
  }
  else if(et==ET_TET) {
//     for (auto k : Range(n))
//        for (auto j : Range(n-k))
//             for (auto i : Range(n-j-k))
//               ir.Append(IntegrationPoint(1.0-i*h-j*h-k*h, i*h, j*h));

    // TODO: simplify order of points?? (need to adapt generated interpolation code in shaders)
      for (auto k : Range(n))
          for (auto j : Range(n-k))
              for (auto i : Range(n-k-j))
                      ir.Append(IntegrationPoint(i*h, j*h, k*h));
  }
  else if(et==ET_PRISM) {
      for (auto k : Range(n))
          for (auto j : Range(n))
              for (auto i : Range(n-j))
                  ir.Append(IntegrationPoint(i*h, j*h, k*h));
  }
  else if(et==ET_HEX) {
      for (auto k : Range(n))
          for (auto j : Range(n))
              for (auto i : Range(n))
                      ir.Append(IntegrationPoint(i*h, j*h, k*h));
  }
  else if(et==ET_PYRAMID) {
      for (auto k : Range(n))
          for (auto j : Range(n-k))
              for (auto i : Range(n-k))
                  ir.Append(IntegrationPoint(i*h, j*h, k*h));
  }
  else {
      throw Exception("GetReferenceRule(): unknown element type");
  }

  return ir;
}

inline map<ELEMENT_TYPE, IntegrationRule> GetReferenceRules( int order, int subdivision )
{
  int n = (order)*(subdivision+1)+1;
  const double h = 1.0/(n-1);
  map<ELEMENT_TYPE, IntegrationRule> res;
  {
      IntegrationRule ir;
      for (auto i : Range(n)) {
          ir.Append(IntegrationPoint(1.0-i*h, 0, 0.0));
      }
      res[ET_SEGM] = std::move(ir);
  }
  {
      IntegrationRule ir;
      for (auto j : Range(n))
          for (auto i : Range(n)) {
              if(i+j<n) // skip i+j>=n for trigs
                  ir.Append(IntegrationPoint(i*h, j*h, 0.0));
          }
      res[ET_TRIG] = std::move(ir);
  }
  {
      IntegrationRule ir;
      for (auto j : Range(n))
          for (auto i : Range(n))
              ir.Append(IntegrationPoint(i*h, j*h, 0.0));
      res[ET_QUAD] = std::move(ir);
  }
  {
      IntegrationRule ir;
      for (auto k : Range(n))
          for (auto j : Range(n-k))
              for (auto i : Range(n-k-j))
                      ir.Append(IntegrationPoint(i*h, j*h, k*h));
      res[ET_TET] = std::move(ir);
  }
//   else if(et==ET_PRISM) {
//       for (auto k : Range(n))
//           for (auto j : Range(n))
//               for (auto i : Range(n-j))
//                   ir.Append(IntegrationPoint(i*h, j*h, k*h));
//   }
//   else if(et==ET_HEX) {
//       for (auto k : Range(n))
//           for (auto j : Range(n))
//               for (auto i : Range(n))
//                       ir.Append(IntegrationPoint(i*h, j*h, k*h));
//   }
//   else if(et==ET_PYRAMID) {
//       for (auto k : Range(n))
//           for (auto j : Range(n-k))
//               for (auto i : Range(n-k))
//                   ir.Append(IntegrationPoint(i*h, j*h, k*h));
//   }
//   else {
//       throw Exception("GetReferenceRule(): unknown element type");
//   }

  return res;
}


template<int S, int R>
BaseMappedIntegrationRule &T_GetMappedIR (IntegrationRule & ir, ElementTransformation & eltrans, LocalHeap &lh ) {
    void *p = lh.Alloc(sizeof(MappedIntegrationRule<S,R>));
    return *new (p) MappedIntegrationRule<S,R> (ir, eltrans, lh);
}

template<int S, int R>
SIMD_BaseMappedIntegrationRule &T_GetMappedIR (SIMD_IntegrationRule & ir, ElementTransformation & eltrans, LocalHeap &lh ) {
    void *p = lh.Alloc(sizeof(SIMD_MappedIntegrationRule<S,R>));
    return *new (p) SIMD_MappedIntegrationRule<S,R> (ir, eltrans, lh);
}


template<typename TIR>
auto &GetMappedIR (TIR & ir, int dim, VorB vb, ElementTransformation & eltrans, LocalHeap &lh ) {
    if(dim==1) {
        if(vb==VOL) return T_GetMappedIR<1,1>(ir, eltrans, lh);
    }
    if(dim==2) {
        if(vb==BND) return T_GetMappedIR<1,2>(ir, eltrans, lh);
        if(vb==VOL) return T_GetMappedIR<2,2>(ir, eltrans, lh);
    }
    if(dim==3) {
        if(vb==BBND) return T_GetMappedIR<1,3>(ir, eltrans, lh);
        if(vb==BND) return T_GetMappedIR<2,3>(ir, eltrans, lh);
        if(vb==VOL) return T_GetMappedIR<3,3>(ir, eltrans, lh);
    }
    throw Exception("GetMappedIR: unknown dimension/VorB combination: " + ToString(vb) + ","+ToString(dim));
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
                float vreal = 0.0;
                float vimag = 0.0;
                ExtractRealImag( values(i, k/n), k%n, vreal, vimag );
                auto index = getIndex(k,i);
                values_real[index] = vreal;
                if(is_complex) {
                  values_imag[index] = vimag;
                  min[i] = min2(min[i], sqrt(vreal*vreal+vimag+vimag));
                  max[i] = max2(max[i], sqrt(vreal*vreal+vimag+vimag));
                }
                else {
                  min[i] = min2(min[i], vreal);
                  max[i] = max2(max[i], vreal);
                }
            }
        }
    }
    else
    {
        FlatMatrix<TSCAL> values(mir.Size(), ncomps, lh);
        cf.Evaluate(mir, values);
        for (auto k : Range(nip)) {
            for (auto i : Range(ncomps)) {
                float vreal = 0.0;
                float vimag = 0.0;
                ExtractRealImag( values(k,i), 0, vreal, vimag );
                auto index = getIndex(k,i);
                values_real[index] = vreal;
                if(is_complex) {
                  values_imag[index] = vimag;
                  min[i] = min2(min[i], sqrt(vreal*vreal+vimag+vimag));
                  max[i] = max2(max[i], sqrt(vreal*vreal+vimag+vimag));
                }
                else {
                  min[i] = min2(min[i], vreal);
                  max[i] = max2(max[i], vreal);
                }
            }
        }
    }

}

PYBIND11_MODULE(ngui, m) {
  py::class_<ElementInformation>(m, "ElementInformation", py::dynamic_attr())
    .def_readwrite("data",    &ElementInformation::data)
    .def_readwrite("size",    &ElementInformation::size)
    .def_readwrite("type",    &ElementInformation::type)
    .def_readwrite("curved",  &ElementInformation::curved)
    .def_readwrite("nverts",  &ElementInformation::nverts)
    .def_readwrite("dim",  &ElementInformation::dim)
    .def_readwrite("nelements",  &ElementInformation::nelements)
    ;
  m.def("SetLocale", []()
        {
          setlocale(LC_NUMERIC,"C");
        });
  m.def("GetReferenceRule", GetReferenceRule);
  m.def("GetValues", [] (shared_ptr<ngfem::CoefficientFunction> cf, shared_ptr<ngcomp::MeshAccess> ma, VorB vb, int subdivision, int order) {
            auto tm = task_manager;
            task_manager = nullptr;
            LocalHeap lh(10000000, "GetValues");
            int dim = ma->GetDimension();
            if(vb==BND) dim-=1;

            map<ELEMENT_TYPE, IntegrationRule> irs = GetReferenceRules( order, subdivision );
            map<ELEMENT_TYPE, SIMD_IntegrationRule> simd_irs;
            for (auto & p : irs ) {
              simd_irs[p.first] = p.second;
            }
            typedef std::pair<ELEMENT_TYPE,bool> T_ET;
            map<T_ET, Array<float>> values_real;
            map<T_ET, Array<float>> values_imag;

            int ncomps = cf->Dimension();
            Array<float> min(ncomps);
            Array<float> max(ncomps);
            min = std::numeric_limits<float>::max();
            max = std::numeric_limits<float>::lowest();

            map<T_ET, std::atomic<int>> element_counter;
            map<T_ET, std::atomic<int>> element_index;
            for (auto et : {ET_POINT, ET_SEGM, ET_TRIG, ET_QUAD, ET_TET, ET_PRISM, ET_PYRAMID, ET_HEX}) {
              for (auto curved : {false, true}) {
                element_counter[T_ET{et,curved}] = 0;
                element_index[T_ET{et,curved}] = 0;
              }
            }
            ma->IterateElements(vb, lh,[&](auto el, LocalHeap& mlh) {
                auto et = el.GetType();
                element_counter[T_ET{et,el.is_curved}]++;
            });

            bool use_simd = true;
            ma->IterateElements(vb, lh,[&](auto el, LocalHeap& mlh) {
                FlatArray<float> min_local(ncomps, mlh);
                FlatArray<float> max_local(ncomps, mlh);
                auto curved = el.is_curved;

                auto et = el.GetType();
                auto & ir = irs[et];
                auto & simd_ir = simd_irs[et];
                auto &vals_real = values_real[T_ET{et,curved}];
                auto &vals_imag = values_imag[T_ET{et,curved}];
                int nip = irs[et].GetNIP();
                int values_per_element = nip*ncomps;
                size_t first = vals_real.Size();
                size_t next = first + values_per_element;
                vals_real.SetSize(next);
                if(cf->IsComplex()) vals_imag.SetSize(next);
                ElementTransformation & eltrans = ma->GetTrafo (el, mlh);
                if(use_simd)
                  {
                    try
                      {
                        auto & mir = GetMappedIR( simd_ir, ma->GetDimension(), vb, eltrans, mlh );
                        if(cf->IsComplex())
                          GetValues<SIMD<Complex>>( *cf, mlh, mir, vals_real.Range(first,next), vals_imag.Range(first,next), min_local, max_local);
                        else
                          GetValues<SIMD<double>>( *cf, mlh, mir, vals_real.Range(first,next), vals_imag, min_local, max_local);
                      }
                    catch(ExceptionNOSIMD e)
                      {
                        use_simd = false;
                      }
                  }
                if(!use_simd)
                  {
                    ElementTransformation & eltrans = ma->GetTrafo (el, mlh);
                    auto & mir = GetMappedIR( ir, ma->GetDimension(), vb, eltrans, mlh );
                    if(cf->IsComplex())
                      GetValues<Complex>( *cf, mlh, mir, vals_real.Range(first,next), vals_imag.Range(first,next), min_local, max_local);
                    else
                      GetValues<double>( *cf, mlh, mir, vals_real.Range(first,next), vals_imag, min_local, max_local);
                  }
                for (auto i : Range(ncomps)) {
                    float expected = min[i];
                    while (min_local[i] < expected)
                        AsAtomic(min[i]).compare_exchange_weak(expected, min_local[i], std::memory_order_relaxed, std::memory_order_relaxed);
                    expected = max[i];
                    while (max_local[i] > expected)
                        AsAtomic(max[i]).compare_exchange_weak(expected, max_local[i], std::memory_order_relaxed, std::memory_order_relaxed);
                }


              });
          py::gil_scoped_acquire ac;
          py::dict res_real;
          py::dict res_imag;
          for (auto &p : irs) {
              auto et = p.first;
              for (auto curved : {false, true}) {
                if (values_real[T_ET{et,curved}].Size()>0) {
                  res_real[py::make_tuple(et,curved)] = MoveToNumpyArray(values_real[T_ET{et,curved}]);
                    if(cf->IsComplex())
                      res_imag[py::make_tuple(et,curved)] = MoveToNumpyArray(values_imag[T_ET{et,curved}]);
                  }
                }
          }
          py::dict res;
          res["min"] = MoveToNumpyArray(min);
          res["max"] = MoveToNumpyArray(max);
          res["real"] = res_real;
          res["imag"] = res_imag;
          task_manager = tm;
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

        LocalHeap lh(1000000, "GetMeshData");

        std::map<VorB, py::list> element_data;
        ElementInformation points(0, ET_POINT);
        points.nelements = ma->GetNV();
        element_data[getVB(ma->GetDimension())].append(py::cast(points));

        ElementInformation edges(4, ET_SEGM);

        if(ma->GetDimension()>=2) {
            // collect edges
            ElementInformation els(4, ET_SEGM);
            edges.data.SetAllocSize(ma->GetNEdges()*edges.size);
            // Edges of mesh (skip this for dim==1, in this case edges are treated as volume elements below)
            for (auto nr : Range(ma->GetNEdges())) {
                auto pair = ma->GetEdgePNums(nr);
                edges.data.Append({int(nr), int(-1), int(pair[0]), int(pair[1])});
            }
        }

        ElementInformation periodic_vertices(4, ET_SEGM);
        int n_periodic_vertices = ma->GetNPeriodicNodes(NT_VERTEX);
        periodic_vertices.nelements = n_periodic_vertices;
        periodic_vertices.data.SetAllocSize(periodic_vertices.nelements*periodic_vertices.size);
        for(auto idnr : Range(ma->GetNPeriodicIdentifications()))
            for (const auto& pair : ma->GetPeriodicNodes(NT_VERTEX, idnr))
                periodic_vertices.data.Append({idnr, -1, pair[0],pair[1]});

        if(ma->GetDimension()>=1) {
            ElementInformation edges[2] = { {4, ET_SEGM}, {5, ET_SEGM, true } };

            // 1d Elements
            VorB vb = getVB(ma->GetDimension()-1);
            for (auto el : ma->Elements(vb)) {
                auto verts = el.Vertices();
                auto &ei = edges[el.is_curved];

                ei.nelements++;
                ei.data.Append({int(el.Nr()), int(el.GetIndex()), int(verts[0]), int(verts[1])});
                if(el.is_curved) {
                    ei.data.Append(vertices.Size()/3);

                    HeapReset hr(lh);
                    ElementTransformation & eltrans = ma->GetTrafo (el, lh);
                    IntegrationRule &ir = GetP2Rule(el.GetType());
                    auto & mir = GetMappedIR( ir, ma->GetDimension(), vb, eltrans, lh );
                    // normals of corner vertices
                    for (auto j : ngcomp::Range(2)) {
                        auto p = static_cast<DimMappedIntegrationPoint<1>&>(mir[j]);
                        auto n = p.GetNV();
                        for (auto i : Range(3))
                            vertices.Append(n[i]);
                    }
                    // mapped coordinates of midpoint (for P2 interpolation)
                    auto p = mir[2].GetPoint();
                    for (auto i : Range(3))
                        vertices.Append(p[i]);
                }
            }
            element_data[vb].append(edges[0]);
            element_data[vb].append(edges[1]);
        }
        if(ma->GetDimension()>=2) {
            // 2d Elements
            ElementInformation trigs[2] = { {5, ET_TRIG}, {6, ET_TRIG, true } };
            ElementInformation quads[2] = { {6, ET_QUAD}, {7, ET_QUAD, true } };

            VorB vb = getVB(ma->GetDimension()-2);
            for (auto el : ma->Elements(vb)) {
                auto verts = el.Vertices();
                auto nverts = verts.Size();
                auto &ei = (nverts==3) ? trigs[el.is_curved] : quads[el.is_curved];
                ei.nelements++;
                ei.data.Append(el.Nr());
                ei.data.Append(el.GetIndex());
                for (auto i : Range(nverts))
                    ei.data.Append(verts[i]);

                if(el.is_curved) {
                    ei.data.Append(vertices.Size()/3);
                    HeapReset hr(lh);
                    ElementTransformation & eltrans = ma->GetTrafo (el, lh);
                    IntegrationRule &ir = GetP2Rule(el.GetType());
                    auto & mir = GetMappedIR( ir, ma->GetDimension(), vb, eltrans, lh );
                    // normals of corner vertices
                    for (auto j : ngcomp::Range(nverts)) {
                        Vec<3> n(0,0,1);
                        if(vb==BND)
                            n = static_cast<DimMappedIntegrationPoint<3>&>(mir[j]).GetNV();
                        for (auto i : Range(3))
                            vertices.Append(n[i]);
                    }
                    // mapped coordinates of edge midpoints (for P2 interpolation)
                    for (auto j : ngcomp::Range(nverts,ir.Size())) {
                        auto p = mir[j].GetPoint();
                        for (auto i : Range(3))
                            vertices.Append(p[i]);
                    }
                }
            }

            for (auto i : Range(2)) {
              if(trigs[i].data.Size()) element_data[vb].append(trigs[i]);
              if(quads[i].data.Size()) element_data[vb].append(quads[i]);
            }
        }

        if(ma->GetDimension()==3) {
            ElementInformation tets[2] = { {6, ET_TET}, {7, ET_TET, true } };
            ElementInformation pyramids[2] = { {7, ET_PYRAMID}, {8, ET_PYRAMID, true } };
            ElementInformation prisms[2] = { {8, ET_PRISM}, {9, ET_PRISM, true } };
            ElementInformation hexes[2] = { {10, ET_HEX}, {11, ET_HEX, true } };

            for (auto el : ma->Elements(VOL)) {
                auto verts = el.Vertices();
                auto nverts = verts.Size();
                ElementInformation * pei;
                IntegrationRule &ir = GetP2Rule(el.GetType());

                switch(nverts) {
                  case 4:
                    pei = tets;
                    break;
                  case 5:
                    pei = pyramids;
                    break;
                  case 6:
                    pei = prisms;
                    break;
                  case 8:
                    pei = hexes;
                    break;
                  default:
                    throw Exception("GetMeshData(): unknown element");
                }
                ElementInformation &ei = pei[el.is_curved];
                ei.nelements++;
                ei.data.Append(el.Nr());
                ei.data.Append(el.GetIndex());
                for (auto v : verts)
                    ei.data.Append(v);

                if(el.is_curved) {
                    ei.data.Append(vertices.Size()/3);
                    HeapReset hr(lh);
                    ElementTransformation & eltrans = ma->GetTrafo (el, lh);
                    auto & mir = GetMappedIR( ir, ma->GetDimension(), VOL, eltrans, lh );
                    // mapped coordinates of edge midpoints (for P2 interpolation)
                    for (auto &ip : mir) {
                      auto p = ip.GetPoint();
                      for (auto i : Range(3))
                          vertices.Append(p[i]);
                    }
                }
            }
            for (auto i : Range(2)) {
                if(tets[i].data.Size()) element_data[VOL].append(tets[i]);
                if(pyramids[i].data.Size()) element_data[VOL].append(pyramids[i]);
                if(prisms[i].data.Size()) element_data[VOL].append(prisms[i]);
                if(hexes[i].data.Size()) element_data[VOL].append(hexes[i]);
            }
        }

        py::dict py_eldata;

        py::list py_edges;
        py_edges.append(edges);
        py_eldata["edges"] = py_edges;

        py::list py_periodic_vertices;
        py_periodic_vertices.append(periodic_vertices);
        py_eldata["periodic"] = py_periodic_vertices;

        py_eldata[py::cast(BBBND)] = element_data[BBBND];
        py_eldata[py::cast(BBND)] = element_data[BBND];
        py_eldata[py::cast(BND)] = element_data[BND];
        py_eldata[py::cast(VOL)] = element_data[VOL];

        py_eldata["min"] = min;
        py_eldata["max"] = max;
        py_eldata["vertices"] = MoveToNumpyArray(vertices);
        return py_eldata;
    });

    m.def("GetGeoData", [] (shared_ptr<netgen::NetgenGeometry> geo) -> py::dict
          {
            Array<float> vertices;
            Array<int> trigs;
            Array<float> normals;
            Array<float> min = {std::numeric_limits<float>::max(),
                                     std::numeric_limits<float>::max(),
                                     std::numeric_limits<float>::max()};
            Array<float> max = {std::numeric_limits<float>::lowest(),
                                     std::numeric_limits<float>::lowest(),
                                     std::numeric_limits<float>::lowest()};
            Array<string> surfnames;

            auto csg_geo = dynamic_pointer_cast<netgen::CSGeometry>(geo);
            auto stl_geo = dynamic_pointer_cast<netgen::STLGeometry>(geo);
#ifdef OCCGEOMETRY
            auto occ_geo = dynamic_pointer_cast<netgen::OCCGeometry>(geo);
#endif

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
                int offset_points = 0;
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
                            trigs.Append(triapprox->GetTriangle(j)[k]+offset_points);
                        trigs.Append(triapprox->GetTriangle(j).SurfaceIndex());
                      }
                    offset_points += triapprox->GetNP();
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
#ifdef OCCGEOMETRY
            else if(occ_geo)
              {
                auto box = occ_geo->GetBoundingBox();
                for(auto i : Range(3))
                  {
                    min[i] = box.PMin()[i];
                    max[i] = box.PMax()[i];
                  }
                occ_geo->BuildVisualizationMesh(0.01);
                gp_Pnt2d uv;
                gp_Pnt pnt;
                gp_Vec n;
                gp_Pnt p[3];
                int count = 0;
                for (int i = 1; i <= occ_geo->fmap.Extent(); i++)
                  {
                    surfnames.Append("occ_surface" + to_string(i));
                    auto face = TopoDS::Face(occ_geo->fmap(i));
                    auto surf = BRep_Tool::Surface(face);
                    TopLoc_Location loc;
                    BRepAdaptor_Surface sf(face, Standard_False);
                    BRepLProp_SLProps prop(sf, 1, 1e-5);
                    Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation (face, loc);
                    if (triangulation.IsNull())
                      cout << "cannot visualize face " << i << endl;
                    trigs.SetAllocSize(trigs.Size() + triangulation->NbTriangles()*4);
                    vertices.SetAllocSize(vertices.Size() + triangulation->NbTriangles()*3*3);
                    normals.SetAllocSize(normals.Size() + triangulation->NbTriangles()*3*3);
                    for (auto j : Range(1,triangulation->NbTriangles()+1))
                      {
                        auto triangle = (triangulation->Triangles())(j);
                        for (auto k : Range(1,4))
                          p[k-1] = (triangulation->Nodes())(triangle(k)).Transformed(loc);
                        for(auto k : Range(1,4))
                          {
                            vertices.Append({float(p[k-1].X()), float(p[k-1].Y()), float(p[k-1].Z())});
                            trigs.Append({count, count+1, count+2,i});
                            count += 3;
                            uv = (triangulation->UVNodes())(triangle(k));
                            prop.SetParameters(uv.X(), uv.Y());
                            if (prop.IsNormalDefined())
                              n = prop.Normal();
                            else
                              {
                                gp_Vec a(p[0], p[1]);
                                gp_Vec b(p[0], p[2]);
                                n = b^a;
                              }
                            if (face.Orientation() == TopAbs_REVERSED) n*= -1;
                            normals.Append({float(n.X()), float(n.Y()), float(n.Z())});
                          }
                      }
                  }
              }
#endif
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

    m.def("GetGeometry2dData", [](netgen::SplineGeometry2d& geo)
          {
            py::tuple min_val = py::make_tuple(1e99, 1e99,0);
            py::tuple max_val = py::make_tuple(-1e99, -1e99,0);
            py::list vertices;
            py::list domains;
            int max_bcnr = 0;
            for (auto i : Range(geo.splines.Size()))
              {
                std::vector<netgen::GeomPoint<2>> lst;
                if (geo.splines[i]->GetType().compare("line") == 0)
                  lst = {geo.splines[i]->StartPI(), geo.splines[i]->GetPoint(1)};
                for (auto point : lst)
                  {
                    for(auto i : Range(2))
                      {
                        min_val[i] = min2(min_val[i].cast<double>(), point(i));
                        max_val[i] = max2(max_val[i].cast<double>(), point(i));
                      }
                    for(auto val : {point(0), point(1), 0.})
                      vertices.append(val);
                    int bcnr = geo.GetSpline(i).bc;
                    max_bcnr = max2(max_bcnr, bcnr);
                    domains.append(bcnr);
                    domains.append(geo.GetSpline(i).leftdom);
                    domains.append(geo.GetSpline(i).rightdom);
                  }
              }
            py::list bcnames;
            for (auto i : Range(1,max_bcnr+1))
              bcnames.append(geo.GetBCName(i));
            return py::make_tuple(vertices, domains, min_val, max_val, bcnames);
          });
    m.def("GetMaterialName", [](netgen::SplineGeometry2d& geo, int matnr) -> string
          {
            char* mat;
            geo.GetMaterial(matnr, mat);
            if (mat)
              return string(mat);
            return "default";
          });
    m.def("GetSegmentData", [](netgen::SplineGeometry2d& geo)
          {
            py::list points, normals, leftdom, rightdom;
            for (int i = 0; i < geo.splines.Size(); i++)
              {
                netgen::GeomPoint<2> point = geo.splines[i]->GetPoint(0.5);
                netgen::Vec<2> normal = geo.GetSpline(i).GetTangent(0.5);
                double temp = normal(0);
                normal(0) = normal(1);
                normal(1) = -temp;
                normal *= 1./sqrt(normal(0)*normal(0) + normal(1)*normal(1));

                leftdom.append(py::cast(geo.GetSpline(i).leftdom));
                rightdom.append(py::cast(geo.GetSpline(i).rightdom));

                points.append(py::make_tuple(point(0),point(1)));
                normals.append(py::make_tuple(normal(0),normal(1)));
              }
            return py::make_tuple(points, normals, leftdom, rightdom);
          });
}
