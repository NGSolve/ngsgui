#include<fem.hpp>
#include<comp.hpp>
#include<l2hofe_impl.hpp>
#include<l2hofefo.hpp>

#include<pybind11/pybind11.h>
#include<pybind11/stl_bind.h>
#include<pybind11/numpy.h>
#include<regex>

using namespace ngfem;

namespace genshader {
    template <ELEMENT_TYPE ET, typename BASE>
      class MyFEL : public BASE {
        public:
          using BASE::ndof;
          using BASE::vnums;
          using BASE::order;
          using BASE::order_inner;
          using BASE::GetFaceSort;
          using BASE::GetEdgeSort;
          MyFEL(int order) : BASE(order) {}
          MyFEL() : BASE() {}
          template<typename Tx, typename TFA>  
            INLINE void MyCalcShape (TIP<ET_trait<ET>::DIM,Tx> ip, TFA & shape) const
              {
                BASE::T_CalcShape(ip, shape);
              }
      };

    static std::vector<string> expressions;

    struct CCode {
        static int find( std::vector<string> &v, string val ){ 
            int i = 0;
            for(auto &s : v) {
                if(s==val)
                  return i;
                i++;
            }
            return -1;
        }

        static string strip(string s) {
            int n = s.size();
            if(n<=1) return s;
            if(s[0] == '(' && s[n-1] == ')') return strip(s.substr(1,n-2));
            return s;
        }
        mutable string s;

        static CCode Par(const CCode &c) {
            return c.s;
        }

        void Check() {
            static string int_num = "var[0-9]*";
            static regex pattern(int_num);
            if(s=="") return;
//             s = "("+s+")";
            int index = find( expressions, strip(s));
            if(index>=0) {
                s = "var"+ToString(index);
            }
            else {
                if(!regex_match(strip(s), pattern)) {
                    expressions.push_back(strip(s));
                    s = "var"+ToString(expressions.size()-1);
                }
            }
        }

        CCode(const CCode &c) : 
          s(c.s)
        {
          Check();
        }

        CCode(string as = "") : s(as) {
            Check();
        }

        CCode(double val) {
            std::stringstream str;
            str << fixed << setprecision( 15 ) << val;
            s = str.str();
            Check();
        }

        virtual CCode operator +(const CCode &c) { return CCode(s+'+'+c.s); }
        virtual CCode operator -(const CCode &c) { return CCode(s+'-'+c.s); }
        virtual CCode operator *(const CCode &c) { return CCode(s+'*'+c.s); }
        virtual void operator +=(const CCode &c) { *this = *this+c; }
        virtual void operator *=(const CCode &c) { *this = *this*c; }
        virtual CCode &operator=(const CCode &c) {
            s = c.s;
            return *this;
        }
        virtual CCode operator /(const CCode &c) { return CCode(s+'/'+c.s); }
//         virtual void operator -=(const CCode &c) { *this = *this-c; }
//         virtual void operator /=(const CCode &c) { *this = *this/c; }
    };

    CCode operator -(int val, const CCode &c) { return CCode(1.0*val)-c; }
    CCode operator *(int val, const CCode &c) { return CCode(1.0*val)*c; }
    CCode operator -(double val, const CCode &c) { return CCode(val)-c; }
    CCode operator *(double val, const CCode &c) { return CCode(val)*c; }

    ostream &operator <<(ostream & s, const CCode &c) {
        s << c.s;
        return s;
    }

    string GenerateCode(int order) {
        auto genCode = [&] (const auto &fel, auto ip, string elname) -> string
        {
            Array<CCode> shape(fel.ndof);
            fel.MyCalcShape (ip, shape);

            stringstream f;
            f << 
              "float Eval" << elname << "( float x, float y, float z )\n"
              "{                             \n"
              " float result = 0.0;" << endl;

            stringstream ss;
            fel.MyCalcShape (ip, SBLambda([&] (int i, auto c) {
                                          ss << "result += texelFetch( coefficients, inData.element*"+ToString(fel.ndof) + "+"  + ToString(i) + ").r * " + c.s << ";" << endl;
                                          }));

            int i = 0;
            for(auto &s : expressions)
              f << "float var" << ToString(i++) << " = " <<  s << ";" << endl;
            f << ss.str() << endl;
            f << "return result;" << endl;
            f << "}" << endl;
            return f.str();
        };

        string code;

        {
            expressions.clear(); CCode x("x"); CCode y("y"); TIP<2,CCode> ip(x,y);
            code += genCode(MyFEL<ET_TRIG, L2HighOrderFE<ET_TRIG>>(order), ip, "TRIG");
        } {
            expressions.clear(); CCode x("x"); CCode y("y"); TIP<2,CCode> ip(x,y);
            code += genCode(MyFEL<ET_QUAD, L2HighOrderFE<ET_QUAD>>(order), ip, "QUAD");
        } {
            expressions.clear(); CCode x("x"); CCode y("y"); CCode z("z"); TIP<3,CCode> ip(x,y,z);
            code += genCode(MyFEL<ET_TET, L2HighOrderFE<ET_TET>>(order), ip, "TET");
        } {
            expressions.clear(); CCode x("x"); CCode y("y"); CCode z("z"); TIP<3,CCode> ip(x,y,z);
            code += genCode(MyFEL<ET_HEX, L2HighOrderFE<ET_HEX>>(order), ip, "HEX");
        } {
            expressions.clear(); CCode x("x"); CCode y("y"); CCode z("z"); TIP<3,CCode> ip(x,y,z);
            code += genCode(MyFEL<ET_PYRAMID, L2HighOrderFE<ET_PYRAMID>>(order), ip, "PYRAMID");
        } {
            expressions.clear(); CCode x("x"); CCode y("y"); CCode z("z"); TIP<3,CCode> ip(x,y,z);
            code += genCode(MyFEL<ET_PRISM, L2HighOrderFE<ET_PRISM>>(order), ip, "PRISM");
        }
        return code;
    }

}
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

    m.def("GenerateShader", &genshader::GenerateCode);
    m.def("GetTetData", [] (shared_ptr<ngcomp::MeshAccess> ma) {
        LocalHeap lh(1000000, "gettetdata");
        ngstd::Array<float> coordinates;
        ngstd::Array<float> bary_coordinates;
        ngstd::Array<int> element_number;

        size_t ntets = ma->GetNE();


        coordinates.SetAllocSize(4*ntets*3);
        bary_coordinates.SetAllocSize(4*ntets*3);
        element_number.SetAllocSize(4*ntets);

        for (auto i : ngcomp::Range(ntets)) {
          auto ei = ElementId(VOL, i);
          auto el = ma->GetElement(ei);

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

                auto EmitElement = [&] (int a, int b, int c, int d) {
                    int pi[4];
                    pi[0] = a;
                    pi[1] = b;
                    pi[2] = c;
                    pi[3] = d;


                    for (auto i : Range(4)) {
                      element_number.Append(ei.Nr());
                      for (auto k : Range(3)) {
                          coordinates.Append(points[pi[i]][k]);
                          bary_coordinates.Append(ref_coords_sorted[pi[i]][k]);
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
                }
            }
            element_number.Append(i);
            element_number.Append(i);
            element_number.Append(i);
            element_number.Append(i);
          }
        }


        return py::make_tuple(
            element_number.Size()/4,
            MoveToNumpyArray(coordinates),
            MoveToNumpyArray(bary_coordinates),
            MoveToNumpyArray(element_number)
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
    m.def("GetFaceData", [] (shared_ptr<ngcomp::MeshAccess> ma) {
        ngstd::Array<float> coordinates;
        ngstd::Array<float> bary_coordinates;
        ngstd::Array<int> element_number;
        size_t ntrigs;

        Vector<> min(3);
        min = std::numeric_limits<double>::max();
        Vector<> max(3);
        max = std::numeric_limits<double>::lowest();

        auto addVertices = [&] (auto verts) {
            ArrayMem<int,3> sorted_vertices{0,1,2};
            ArrayMem<int,3> unsorted_vertices{verts[0], verts[1], verts[2]};

            BubbleSort (unsorted_vertices, sorted_vertices);
            for (auto j : ngcomp::Range(3)) {
                auto v = ma->GetPoint<3>(verts[j]);
                for (auto k : Range(3)) {
                  coordinates.Append(v[k]);
                  bary_coordinates.Append( unsorted_vertices[k]==verts[j] ? 1.0 : 0.0 );

                  min[k] = min2(min[k], v[k]);
                  max[k] = max2(max[k], v[k]);
                }
            }
        };

        if(ma->GetDimension()==2)
        {
            ntrigs = ma->GetNE();
            element_number.SetSize(3*ntrigs);
            for (auto i : ngcomp::Range(ntrigs)) {
                addVertices(ma->GetElement(ElementId( VOL, i)).Vertices());
                element_number[3*i+0] = i;
                element_number[3*i+1] = i;
                element_number[3*i+2] = i;
            }
        }
        else if(ma->GetDimension()==3)
        {
            ntrigs = ma->GetNSE();
            element_number.SetSize(3*ntrigs);

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
                element_number[3*sel.Nr()+0] = elnums[0];
                element_number[3*sel.Nr()+1] = elnums[0];
                element_number[3*sel.Nr()+2] = elnums[0];
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
            min, max
        );
      },py::call_guard<py::gil_scoped_release>());

}
