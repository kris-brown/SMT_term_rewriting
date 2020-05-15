#ifndef ASTEXTRA
#define ASTEXTRA
#include<vector>
#include <cvc4/api/cvc4cpp.h>
#include "theory.hpp"

namespace CVC = CVC4::api;
typedef std::vector<CVC::Term> Vt;

typedef std::vector<void (*)()> Vf;
typedef std::vector<int> Vi;
typedef std::vector<Vi> Vvi;
typedef std::vector<Vvi> Vvvi;
typedef std::map<int, CVC::Term> Tmap;
typedef std::tuple<CVC::Sort,CVC::Sort,CVC::Sort,CVC::Sort,CVC::Datatype,CVC::Datatype,CVC::Datatype,CVC::Term,CVC::Term,CVC::Term,CVC::Term,CVC::Term,CVC::Term,CVC::Term,CVC::Term,CVC::Term,CVC::Term,CVC::Term,CVC::Term,CVC::Term,CVC::Term,CVC::Term,CVC::Term,Vt,Vi,std::map<Vi,CVC::Term>,Tmap> CDTuple; // Only for output of create_datatype()
typedef std::tuple<CVC::Sort, CVC::Sort, CVC::Sort, CVC::Sort, CVC::Datatype, CVC::Datatype,CVC::Datatype,CVC::Term,CVC::Term,CVC::Term,CVC::Term,CVC::Term,CVC::Term,CVC::Term,Vt,Vi,Vt,Vt> STuple; // Only for output of setup()


// Cartesian product of vector of vectors, taken from: https://stackoverflow.com/a/5279601
void cart_product(
    Vvi& rvvi, // final result
    Vi&  rvi, // current result
    Vvi::const_iterator me, // current input
    Vvi::const_iterator end);// final input

// Path vector generation: we want (arity ^ depth) number of paths
Vvi paths_n(const int&  depth, const int& arity);

// All paths of length 1, 2, ... depth
Vvvi all_paths(const int&  depth, const int& arity);


// Create AST, Rule, Path datatypes, return lots of associated sorts/constructors/example terms
CDTuple create_datatypes(CVC::Solver & slv,
                      const Theory & t,
                      const int & depth);

// safe access to AST selectors, shorthand constructors for terms of any arity, replacing top-level arguments of a term.
std::tuple<Vt,Vt,CVC::Term> arity_funcs(
    CVC::Solver & slv,
    const int & arity,
    const CVC::Sort & Int,
    const CVC::Sort & astSort,
    const CVC::Datatype & astDT,
    const CVC::Term & astCon,
    const CVC::Term & nInt,
    const CVC::Term & xTerm,
    const CVC::Term & yTerm,
    const CVC::Term & noneterm,
    const CVC::Term & noneX,
    const CVC::Term & nodeX,
    const Vt & as);


// For each rule, a pattern predicate to determine if rule is valid and a function to perform the rewrite, along with a top-level rewrite function which combines the other two.
std::tuple<Tmap, Tmap, CVC::Term> rule_funcs(
    CVC::Solver & slv,
    const Theory & t,
    const CVC::Sort &  astSort,
    const CVC::Sort &  Int,
    const CVC::Term &  xTerm,
    const CVC::Term &  rTerm,
    const CVC::Term &  errterm,
    const CVC::Term &  sInt,
    const CVC::Term &  node,
    const std::map<int,CVC::Term> & rulecon,
    const Vt &  as,
    const Vt &  asts,
    const Vi & rules);

std::tuple<CVC::Term,Vt> rewrite_funcs(
    CVC::Solver & slv,
    const CVC::Sort & astSort,
    const CVC::Sort & ruleSort,
    const CVC::Sort & pathSort,
    const CVC::Term & xTerm,
    const CVC::Term & rTerm,
    const CVC::Term & pTerm,
    const CVC::Term & sInt,
    const CVC::Term & astX,
    const CVC::Term & errterm,
    const CVC::Term & replaceAt,
    const CVC::Term & rewriteTop,
    const CVC::Term & getAt);

// Call create_datatypes, arity_funcs, path_funcs, rewrite_funs
STuple setup(CVC::Solver & slv,
             const Theory & t,
             const int & depth,
             const bool incl_rewrite=true,
             const bool incl_rule=true,
             const bool incl_path=true,
             const bool incl_arity=true);

// e.g. ast3 : (INT, AST, AST, AST) -> AST
// = (LAMBDA(n:INT, x0:AST, x1:AST, x2:AST): ast(n, x0, x1, x2, None));
CVC::Term ast_fun(CVC::Solver & slv,
                  const Vt & xs,
                  const CVC::Term & nInt,
                  const CVC::Sort & astSort,
                  const CVC::Datatype & astDT,
                  const CVC::Term & noneterm,
                  const int & i);

// Safe access to the selectors of the AST constructor
// e.g.    A2 : T -> T   = LAMBDA (x:T): IF x = None THEN None ELSE a2(x) ENDIF;
// Also: Node : T -> INT = LAMBDA (x:T): IF x = None THEN 0    ELSE node(x) ENDIF;
CVC::Term a_fun(CVC::Solver & slv,
                const CVC::Term & x,
                const CVC::Sort & astSort,
                const CVC::Sort & intSort,
                const CVC::Datatype & astDT,
                const CVC::Term & noneterm,
                const CVC::Term & noneX,
                const int & i);

// Replace a top-level argument of a term
// replace2 : (T,T) -> T = LAMBDA (x,y: T):ast(node(x),a0(x),a1(x),y,a3(x));
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

/* Replace a specific subnode of a term
replaceP321 : (T, T) -> T = LAMBDA(x,y:T): replace3(x, replace2(a3(x), replace1(a2(a3(x)), y)));
*/
CVC::Term replP_fun(CVC::Solver & slv,
                const CVC::Term & x,
                const CVC::Term & y,
                const CVC::Sort & astSort,
                const Vt & as,
                const Vt & reps,
                const Vi & p);

// Apply a1(a3(a4(...(x)))) specified by vector pth
CVC::Term pathterm(const CVC::Solver & slv,
                   const CVC::Term & root,
                   const Vt & selectors,
                   const Vi & pth);

// Create a predicate on terms which is true if it pattern-matches the first Expr of a rule
CVC::Term pat_fun(const CVC::Solver & slv,
                  const CVC::Term & x,
                  const CVC::Term & node,
                  const int & r,
                  const Vt & selectors,
                  const Theory & thry,
                  const std::map<std::string,int> & symcode);

// Construct a term, making reference to subexpressions of some existing term x when possible

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

// Enumeration of variables in x that do not appear in y

std::map<std::string,int> mk_freevar(Expr x, Expr y) ;

// Concretely apply a rewrite rule to construct a new term from an old one
CVC::Term rterm_fun(const CVC::Solver & slv,
                    const CVC::Term & x,
                    const CVC::Term & s,
                    const CVC::Sort & astSort,
                    const int & r,
                    const Theory & thry,
                    const Vt & asts,
                    const Vt & selectors,
                    std::map<std::string,int> & symcode);

// A function which applies i rewrites in sequence
CVC::Term rw_fun(const CVC::Solver & slv,
                 const int & imax,
                 const CVC::Sort & astSort,
                 const CVC::Term & rewrite,
                 const Vt & rs,
                 const Vt & ps);
#endif