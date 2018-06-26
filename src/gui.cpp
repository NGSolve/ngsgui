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

struct ElementInformation {
    ElementInformation() = default;
    ElementInformation( size_t size_, ELEMENT_TYPE type_, bool curved_=false)
      : size(size_), type(type_), curved(curved_) {}
    Array<int> data; // the data that will go into the gpu texture buffer
    size_t size;   // integer entries for each element (vertices, curved_index, number, material/boundary index)
    ELEMENT_TYPE type;
    bool curved;
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
        cout << "init integration rules" << endl;

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
//     for (auto et : {ET_TRIG, ET_QUAD, ET_TET, ET_HEX, ET_PRISM, ET_PYRAMID}) {
//         cout << "rule for " << et << endl;
//         cout << GetReferenceRule(et, 1, 1 ) << endl;
//     }
  py::class_<ElementInformation>(m, "ElementInformation", py::dynamic_attr())
    .def_readwrite("data",    &ElementInformation::data)
    .def_readwrite("size",    &ElementInformation::size)
    .def_readwrite("type",    &ElementInformation::type)
    .def_readwrite("curved",  &ElementInformation::curved)
    ;
  m.def("SetLocale", []()
        {
          setlocale(LC_NUMERIC,"C");
        });
  m.def("GetValues2", [] (shared_ptr<ngfem::CoefficientFunction> cf, shared_ptr<ngcomp::MeshAccess> ma, VorB vb, int subdivision, int order) {
            LocalHeap lh(10000000, "GetValues");
            int dim = ma->GetDimension();
            if(vb==BND) dim-=1;

            map<ELEMENT_TYPE, IntegrationRule> irs = GetReferenceRules( order, subdivision );
            map<ELEMENT_TYPE, SIMD_IntegrationRule> simd_irs;
            for (auto & p : irs ) {
              simd_irs[p.first] = p.second;
            }
            map<ELEMENT_TYPE, Array<float>> values_real;
            map<ELEMENT_TYPE, Array<float>> values_imag;

            int ncomps = cf->Dimension();
            Array<float> min(ncomps);
            Array<float> max(ncomps);
            min = std::numeric_limits<float>::max();
            max = std::numeric_limits<float>::lowest();

            bool use_simd = true;
            ma->IterateElements(vb, lh,[&](auto el, LocalHeap& mlh) {
                FlatArray<float> min_local(ncomps, mlh);
                FlatArray<float> max_local(ncomps, mlh);

                auto et = el.GetType();
                auto & ir = irs[et];
                auto & simd_ir = simd_irs[et];
                auto &vals_real = values_real[et];
                auto &vals_imag = values_imag[et];
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
              if (values_real[et].Size()>0) {
                  py::dict v;
                  res_real[py::cast(et)] = MoveToNumpyArray(values_real[et]);
                  if(cf->IsComplex())
                    res_imag[py::cast(et)] = MoveToNumpyArray(values_imag[et]);
              }
          }
          py::dict res;
          res["min"] = MoveToNumpyArray(min);
          res["max"] = MoveToNumpyArray(max);
          res["real"] = res_real;
          res["imag"] = res_imag;
          return res;
      },py::call_guard<py::gil_scoped_release>());
  m.def("GetValues", [] (shared_ptr<ngfem::CoefficientFunction> cf, shared_ptr<ngcomp::MeshAccess> ma, VorB vb, int subdivision, int order) {
            LocalHeap lh(10000000, "GetValues");
            int dim = ma->GetDimension();
            if(vb==BND) dim-=1;

            int ncomps = cf->Dimension();
            Array<float> min(ncomps);
            Array<float> max(ncomps);
            min = std::numeric_limits<float>::max();
            max = std::numeric_limits<float>::lowest();

            IntegrationRule ir = GetReferenceRule( dim==2?ET_TRIG:ET_TET, order, subdivision );
            SIMD_IntegrationRule simd_ir(ir);
            int nip = ir.GetNIP();

            int values_per_element = nip*ncomps;

            Array<float> res_real;
            Array<float> res_imag;

            res_real.SetSize(ma->GetNE(vb)*values_per_element); // two entries for global min/max
            if(cf->IsComplex())
                res_imag.SetSize(ma->GetNE(vb)*values_per_element); // two entries for global min/max

            bool use_simd = true;
            ma->IterateElements(vb, lh,[&](auto el, LocalHeap& mlh) {
                FlatArray<float> min_local(ncomps, mlh);
                FlatArray<float> max_local(ncomps, mlh);
                if(use_simd)
                  {
                    try
                      {
                        ElementTransformation & eltrans = ma->GetTrafo (el, mlh);
                        auto & mir = GetMappedIR( simd_ir, ma->GetDimension(), vb, eltrans, mlh );
                        size_t first = el.Nr()*values_per_element;
                        size_t next = (el.Nr()+1)*values_per_element;
                        if(cf->IsComplex())
                          GetValues<SIMD<Complex>>( *cf, mlh, mir, res_real.Range(first,next), res_imag.Range(first,next), min_local, max_local);
                        else
                          GetValues<SIMD<double>>( *cf, mlh, mir, res_real.Range(first,next), res_imag, min_local, max_local);
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
                    size_t first = el.Nr()*values_per_element;
                    size_t next = (el.Nr()+1)*values_per_element;
                    if(cf->IsComplex())
                      GetValues<Complex>( *cf, mlh, mir, res_real.Range(first,next), res_imag.Range(first,next), min_local, max_local);
                    else
                      GetValues<double>( *cf, mlh, mir, res_real.Range(first,next), res_imag, min_local, max_local);
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
          py::dict res;
          res["real"] = MoveToNumpyArray(res_real);
          if(cf->IsComplex())
            res["imag"] = MoveToNumpyArray(res_imag);
          res["min"] = MoveToNumpyArray(min);
          res["max"] = MoveToNumpyArray(max);
          return res;
      },py::call_guard<py::gil_scoped_release>());

    m.def("GetMeshData2", [] (shared_ptr<ngcomp::MeshAccess> ma) {
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

        ElementInformation edges(4, ET_SEGM);
        std::map<VorB, py::list> element_data;

        if(ma->GetDimension()>=2) {
            // collect edges
            ElementInformation els(4, ET_SEGM);
            edges.data.SetAllocSize(ma->GetNEdges()*edges.size);
            // Edges of mesh (skip this for dim==1, in this case edges are treated as volume elements below)
            for (auto nr : Range(ma->GetNEdges())) {
                auto pair = ma->GetEdgePNums(nr);
                edges.data.Append({nr, -1, pair[0], pair[1]});
            }
        }

        ElementInformation periodic_vertices(4, ET_SEGM);
        int n_periodic_vertices = ma->GetNPeriodicNodes(NT_VERTEX);
        edges.data.SetAllocSize(n_periodic_vertices*periodic_vertices.size);
        for(auto idnr : Range(ma->GetNPeriodicIdentifications()))
            for (const auto& pair : ma->GetPeriodicNodes(NT_VERTEX, idnr))
                periodic_vertices.data.Append({idnr, -1, pair[0],pair[1]});

        if(ma->GetDimension()>=1) {
            ElementInformation edges[2] = { {4, ET_SEGM}, {5, ET_SEGM, true } };

            // 1d Elements
            VorB vb = ma->GetDimension() == 1 ? VOL : (ma->GetDimension() == 2 ? BND : BBND);
            for (auto el : ma->Elements(vb)) {
                auto verts = el.Vertices();
                auto &ei = edges[el.is_curved];

                ei.data.Append({el.Nr(), el.GetIndex(), verts[0], verts[1]});
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

            VorB vb = ma->GetDimension() == 2 ? VOL : BND;
            for (auto el : ma->Elements(vb)) {
                auto verts = el.Vertices();
                auto nverts = verts.Size();
                auto &ei = (nverts==3) ? trigs[el.is_curved] : quads[el.is_curved];
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

        py_eldata[py::cast(BBND)] = element_data[BBND];
        py_eldata[py::cast(BND)] = element_data[BND];
        py_eldata[py::cast(VOL)] = element_data[VOL];
        return py::make_tuple(MoveToNumpyArray(vertices), py_eldata);
    });


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

        int n_edges = 0;
        int n_edge_elements = 0;
        int n_periodic_vertices = ma->GetNPeriodicNodes(NT_VERTEX);
        int n_surface_elements = 0;
        int n_volume_elements = 0;

        int meshdim = ma->GetDimension();
        switch (meshdim) {
          case 3:
            n_surface_elements = ma->GetNSE();
            n_volume_elements = ma->GetNE();
            n_edges = ma->GetNEdges();
            n_edge_elements = ma->GetNE(BBND);
            break;
          case 2:
            n_surface_elements = ma->GetNE();
            n_edges = ma->GetNEdges();
            n_edge_elements = ma->GetNE(BND);
            break;
          case 1:
            n_edge_elements = ma->GetNE();
            break;
        }


        size_t elsize = edge_elements_size * (n_edge_elements + n_edges + n_periodic_vertices) +
                        surface_elements_size*n_surface_elements +
                        volume_elements_size*n_volume_elements;
        elements.SetAllocSize(2*elsize);

        // additional information for curved (i.e. real curved or non-simplex elements like quads)
        // first  entry: number of vertices (also determines element type)
        // second entry: offset at vertices array for additional float data (normals, edge midpoint coords etc.)
        // other entries: additional vertex indices (4th vertex for quads, 5th and other for prism, pyramid, hex)
        ngstd::Array<int> curve_info;

        size_t size_elements_before = elements.Size();

        if(ma->GetDimension()>=2) {
            // Edges of mesh (skip this for dim==1, in this case edges are treated as volume elements below)
            for (auto nr : Range(n_edges)) {
                auto verts = ma->GetEdgePNums(nr);
                for (auto i : Range(2))
                    elements.Append(verts[i]);
                elements.Append(0); // always use first color now
                elements.Append(-1); // never curved

            }
            assert(elements.Size() == size_elements_before +n_edges*edge_elements_size);
        }
        size_elements_before = elements.Size();
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
                    curve_info.Append(vertices.Size()/3);

                    HeapReset hr(lh);
                    ElementTransformation & eltrans = ma->GetTrafo (el, lh);
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

            assert(elements.Size() == size_elements_before + n_edge_elements*edge_elements_size);
        }
        size_elements_before = elements.Size();
        if(ma->GetDimension()>=1)
          {
            for(auto idnr : Range(ma->GetNPeriodicIdentifications()))
              for (const auto& pair : ma->GetPeriodicNodes(NT_VERTEX, idnr))
                elements.Append({pair[0],pair[1],0,-1});
            assert(elements.Size() == size_elements_before +n_periodic_vertices*edge_elements_size);
          }

        size_elements_before = elements.Size();
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
                    for (auto j : ngcomp::Range(3UL,ir.Size())) {
                        auto p = mir[j].GetPoint();
                        for (auto i : Range(3))
                            vertices.Append(p[i]);
                    }
                }
            }

            assert(elements.Size() == size_elements_before +n_surface_elements*surface_elements_size );
        }

        size_elements_before = elements.Size();
        if(ma->GetDimension()==3) {
            // 3d Elements
            IntegrationRule ir;
//             ir.Append(IntegrationPoint(1,0,0));
//             ir.Append(IntegrationPoint(0,1,0));
//             ir.Append(IntegrationPoint(0,0,1));
//             ir.Append(IntegrationPoint(0,0,0));
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
                    auto & mir = GetMappedIR( ir, ma->GetDimension(), VOL, eltrans, lh );
//                     // normals of corner vertices
//                     for (auto j : ngcomp::Range(4)) {
//                       auto p = static_cast<DimMappedIntegrationPoint<3>&>(mir[j]);
//                       auto n = p.GetNV();
//                       for (auto i : Range(3))
//                           vertices.Append(n[i]);
//                     }
                    // mapped coordinates of edge midpoints (for P2 interpolation)
                    for (auto &ip : mir) {
                      auto p = ip.GetPoint();
                      for (auto i : Range(3))
                          vertices.Append(p[i]);
                    }
                }
            }

            assert(elements.Size() == size_elements_before + n_volume_elements*volume_elements_size);
        }


        elements.SetAllocSize(elements.Size()+curve_info.Size());
        for (auto i : curve_info)
            elements.Append(i);


        py::gil_scoped_acquire ac;
        py::dict res;
        res["vertices"] = MoveToNumpyArray(vertices);
        res["elements"] = MoveToNumpyArray(elements);

        res["n_edge_elements"] = n_edge_elements;
        res["n_edges"] = n_edges;
        res["n_periodic_vertices"] = n_periodic_vertices;
        res["n_surface_elements"] = n_surface_elements;
        res["n_volume_elements"] = n_volume_elements;

        res["edges_offset"] = n_edge_elements;
        res["periodic_vertices_offset"] = n_edge_elements+n_edges;
        res["surface_elements_offset"] = (n_periodic_vertices+n_edge_elements+n_edges) * edge_elements_size;
        res["volume_elements_offset"] = (n_periodic_vertices+n_edge_elements+n_edges) * edge_elements_size +
          n_surface_elements*surface_elements_size;

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
            Array<float> max = {std::numeric_limits<float>::lowest(),
                                     std::numeric_limits<float>::lowest(),
                                     std::numeric_limits<float>::lowest()};
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
