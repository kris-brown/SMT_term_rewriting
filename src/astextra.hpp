#ifndef ASTEXTRA
#define ASTEXTRA
#include<vector>
#include<stdexcept>
typedef std::vector<int> Vi;
typedef std::vector<Vi> Vvi;
typedef std::vector<Vvi> Vvvi;
#include <cvc4/api/cvc4cpp.h>
#include <src/theory.hpp>

namespace CVC = CVC4::api;
typedef std::vector<CVC::Term> Vt;


void cart_product(
    Vvi& rvvi,
    Vi&  rvi,
    Vvi::const_iterator me,
    Vvi::const_iterator end);

Vvi paths_n(const int&  depth, const int& arity);

Vvvi all_paths(const int&  depth, const int& arity);

CVC::Term ast_fun(CVC::Solver & slv,
                  const Vt & xs,
                  const CVC::Term & nInt,
                  const CVC::Sort & astSort,
                  const CVC::Datatype & astDT,
                  const CVC::Term & noneterm,
                  const int & i);

CVC::Term a_fun(CVC::Solver & slv,
                const CVC::Term & x,
                const CVC::Sort & astSort,
                const CVC::Sort & intSort,
                const CVC::Datatype & astDT,
                const CVC::Term & noneterm,
                const CVC::Term & noneX,
                const int & i);

CVC::Term repl_fun(CVC::Solver & slv,
                const CVC::Term & x,
                const CVC::Term & y,
                const CVC::Sort & astSort,
                const CVC::Term & noneterm,
                const CVC::Term & nodeX,
                const CVC::Term & astCon,
                const CVC::Term & noneX,
                const Vt & as,
                const int & arity,
                const int & i);

CVC::Term replP_fun(CVC::Solver & slv,
                const CVC::Term & x,
                const CVC::Term & y,
                const CVC::Sort & astSort,
                const Vt & as,
                const Vt & reps,
                const Vi & p);

CVC::Term pathterm(const CVC::Solver & slv,
                   const CVC::Term & root,
                   const Vt & selectors,
                   const Vi & pth);

CVC::Term pat_fun(const CVC::Solver & slv,
                  const CVC::Term & x,
                  const CVC::Term & node,
                  const int & r,
                  const Vt & selectors,
                  const Theory & thry,
                  const std::map<std::string,int> & symcode);


CVC::Term construct(const CVC::Solver & slv,
                    const Expr & tar,
                    const Vi & currpth,
                    const std::map<size_t,Vi> & srchsh,
                    const std::map<Vi,size_t> & tarhsh,
                    const CVC::Term & x,
                    const CVC::Term & step,
                    const Vt & asts,
                    const Vt & selectors,
                    const std::map<std::string,int> freevar,
                    const std::map<std::string,int> & symcode);


std::map<std::string,int> mk_freevar(Expr x, Expr y) ;

CVC::Term rterm_fun(const CVC::Solver & slv,
                    const CVC::Term & x,
                    const CVC::Term & s,
                    const CVC::Sort & astSort,
                    const int & r,
                    const Theory & thry,
                    const Vt & asts,
                    const Vt & selectors,
                    std::map<std::string,int> & symcode);

CVC::Term rw_fun(const CVC::Solver & slv,
                 const int & imax,
                 const CVC::Sort & astSort,
                 const CVC::Term & rewrite,
                 const Vt & rs,
                 const Vt & ps);
#endif