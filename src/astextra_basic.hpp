#ifndef ASTEXTRABASIC
#define ASTEXTRABASIC
#include<vector>
#include <cvc4/api/cvc4cpp.h>
#include "theory.hpp"

namespace CVC = CVC4::api;
typedef std::vector<int> Vi;
typedef std::vector<Vi> Vvi;
typedef std::vector<Vvi> Vvvi;
typedef std::vector<CVC::Term> Vt;

/**
 * Cartesian product of vector of vectors, taken from: https://stackoverflow.com/a/5279601
 *
 * @param rvvi - final result (gets updated)
 * @param rvi - temporary storage
 * @param me - input vector begin
 * @param end - input vector end
 */
void cart_product(
    Vvi& rvvi, //
    Vi&  rvi, // current result
    Vvi::const_iterator me, // current input
    Vvi::const_iterator end);// final input


/**
 * Path vector generation: we want (arity ^ depth) number of paths
 *
 * @param depth - final result (gets updated)
 * @param arity - number of options to take at each step in path
 * @return - vector of paths, e.g. [[0,0],[0,1],[1,0],[1,1]] for arity=2, depth=2
 */
Vvi paths_n(const int&  depth, const int& arity);


/**
 * All paths of length 1, 2, ... depth
 *
 * @param depth - the largest path length, which we will build up to
 * @param arity - number of options to take at each step in path
 * @return - vector of vector of paths
 */
Vvvi all_paths(const int&  depth, const int& arity);


/**
 * Create instance of None constructor of the AST sort
 *
 * @param solver
 * @param astSort - AST sort from create_datatypes()
 * @return - the CVC term None
 */
CVC::Term noneterm(const CVC::Solver & slv, const CVC::Sort & astSort);

/**
 * Create instance of Error constructor of the AST sort
 *
 * @param solver
 * @param astSort - AST sort from create_datatypes()
 * @return - the CVC term Error
 */
CVC::Term errterm(const CVC::Solver & slv, const CVC::Sort & astSort);

/**
 * Number of AST selectors for the ast constructor
 *
 * @param astSort - AST sort from create_datatypes()
 * @return - the highest i for which there is a selector a_i::AST
 */
int arity(const CVC::Sort & astSort);

/**
 * Apply node selector to an AST term.
 *
 * @param solver
 * @param astSort - AST sort from create_datatypes()
 * @param astSort - input term (must be constructed from AST::ast)
 * @return - a CVC term with the node of the input
 */
CVC::Term node(const CVC::Solver & slv,
              const CVC::Sort & astSort,
              const CVC::Term & x) ;

/**
 * Apply arg selector to an AST term.
 *
 * @param solver
 * @param x - AST CVC4 term
 * @return - a CVC term with the node of the input
 */

CVC::Term getarg(const CVC::Solver & slv,
                const CVC::Term & x,
                const int & i);
/**
 * Construct a term with AST's ast constructor.
 *
 * @param solver
 * @param astDT - AST datatype from create_datatypes()
 * @param n - Value of the node
 * @param xs - arguments of the node
 * @return - the three sorts
 */
CVC::Term ast(const CVC::Solver & slv,
              const CVC::Sort & astSort,
              const CVC::Term & n,
              const Vt & xs={}
              ) ;

/**
 * Replace a top-level argument of an AST.
 *
 * @param solver
 * @param astDT - AST datatype from create_datatypes()
 * @param arg - Which argument to replace
 * @param x - term with an arg to be replaced
 * @param y - term substituted for the arg
 * @return - the modified x term
 */

CVC::Term replace(const CVC::Solver & slv,
                  const int & arg,
                  const CVC::Term & x,
                  const CVC::Term & y);

/**
 * Replace an arbitrary subterm, specified by a path
 *
 * @param solver
 * @param x - term for which we will replace a subterm
 * @param y - replacement term
 * @param pth - Location of subterm
 * @return - CVC4 term representing the substitution result.
 */

CVC::Term replP_fun(const CVC::Solver & slv,
                    const CVC::Term & x,
                    const CVC::Term & y,
                    const Vi & p);
/**
 * Apply selectors a1(a3(a4(...(x)))) specified by a vector
 *
 * @param solver
 * @param astDT - AST datatype from create_datatypes()
 * @param root - term which we will extract a subterm
 * @param pth - Instructions to traverse the AST, e.g. take the 4th arg, then 3rd arg, etc.
 * @return - CVC4 term representing the subterm.
 */
CVC::Term subterm(const CVC::Solver & slv,
                   const CVC::Term & root,
                   const Vi & pth);

/**
 * Create arbitrary positive integer for a variable name.
 *
 * @param s - The input string to be hashed
 * @return an int between 100 and 10000.
 */
int strhash(const std::string & s);


/**
 * Predicate which applies a tester to a term
 *
 * @param solver
 * @param x - a term (AST/Rule/PAth)
 * @param s - a constructor name (e.g. Empty/Error/P132)
 * @returns - A CVC term which evaluates to a bool
*/
CVC::Term test(const CVC::Solver & slv,
               const CVC::Term & x,
               const std::string & s);

/**
 * Top-level call to constructRec
 *
 * @param solver
 * @param astSort - AST sort from create_datatypes()
 * @param tar - term which we will construct with CVC4 api
 * @param src - optional term which we can reference in our construction
 * @param step - Seed to produce distinct free variables, if any
 * @return - CVC4 term representing the subterm.
 */
CVC::Term construct(const CVC::Solver & slv,
                    const CVC::Sort & astSort,
                    const Theory & t,
                    const Expr & tar,
                    const Expr & src=Expr{"?",AppNode,{}},
                    const CVC::Term & src_t=CVC::Term{},
                    const int & step=0);

/**
 * Recursively construct a CVC term from an expression
 *
 * @param solver
 * @param astSort - AST datatype from create_datatypes()
 * @param root - term which we will extract a subterm
 * @param pth - Instructions to traverse the AST, e.g. take the 4th arg, then 3rd arg, etc.
 * @return - CVC4 term representing the subterm.
 */
CVC::Term constructRec(const CVC::Solver & slv,
                       const CVC::Sort & astSort,
                       const Expr & tar,
                       const Vi & currpth,
                       const CVC::Term & src_t,
                       const std::map<size_t,Vi> & srchsh,
                       const std::map<Vi,size_t> & tarhsh,
                       const int & step,
                       const std::map<std::string,int> fv,
                       const std::map<std::string,int> & syms);
#endif