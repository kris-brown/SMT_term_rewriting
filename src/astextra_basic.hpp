#ifndef ASTEXTRABASIC
#define ASTEXTRABASIC
#include<vector>
#include "smt-switch/smt.h"
#include "theory.hpp"

typedef std::vector<int> Vi;
typedef std::vector<Vi> Vvi;
typedef std::vector<Vvi> Vvvi;
typedef std::vector<smt::Term> Vt;

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
 * Create instance of some constructor which takes no arguments
 *
 * @param solver
 * @param srt - Inductive datatype sort
 * @param name - Name of the constructor
 * @return - a CVC term (e.g. None, Empty, Error)
 */

smt::Term unit(const smt::SmtSolver & slv,
               const smt::Sort & srt,
               const std::string &name);
/**
 * Number of AST selectors for the ast constructor
 *
 * @param astSort - AST sort from create_datatypes()
 * @return - the highest i for which there is a selector a_i::AST
 */
int arity(const smt::Sort & astSort);

/**
 * Apply node selector to an AST term.
 *
 * @param solver
 * @param astSort - AST sort from create_datatypes()
 * @param astSort - input term (must be constructed from AST::ast)
 * @return - a CVC term with the node of the input
 */
smt::Term node(const smt::SmtSolver & slv,
              const smt::Term & x) ;

/**
 * Apply arg selector to an AST term.
 *
 * @param solver
 * @param x - AST CVC4 term
 * @return - a CVC term with the node of the input
 */

smt::Term getarg(const smt::SmtSolver & slv,
                const smt::Term & x,
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
smt::Term ast(const smt::SmtSolver & slv,
              const smt::Sort & astSort,
              const smt::Term & n,
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

smt::Term replace(const smt::SmtSolver & slv,
                  const int & arg,
                  const smt::Term & x,
                  const smt::Term & y);

/**
 * Replace an arbitrary subterm, specified by a path
 *
 * @param solver
 * @param x - term for which we will replace a subterm
 * @param y - replacement term
 * @param pth - Location of subterm
 * @return - CVC4 term representing the substitution result.
 */

smt::Term replP_fun(const smt::SmtSolver & slv,
                    const smt::Term & x,
                    const smt::Term & y,
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
smt::Term subterm(const smt::SmtSolver & slv,
                   const smt::Term & root,
                   const Vi & pth);

/**
 * Create arbitrary positive integer for a variable name.
 *
 * @param s - The input string to be hashed
 * @return an int between 100 and 10000.
 */
int64_t strhash(const std::string & s) ;

std::string strhashinv(const int64_t & i);

/**
 * Predicate which applies a tester to a term
 *
 * @param solver
 * @param x - a term (AST/Rule/PAth)
 * @param s - a constructor name (e.g. Empty/Error/P132)
 * @returns - A CVC term which evaluates to a bool
*/
smt::Term test(const smt::SmtSolver & slv,
               const smt::Term & x,
               const std::string & s);
// Negation of test
smt::Term ntest(const smt::SmtSolver & slv,
               const smt::Term & x,
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
smt::Term construct(const smt::SmtSolver & slv,
                    const smt::Sort & astSort,
                    const Theory & t,
                    const Expr & tar,
                    const Expr & src=Expr{"?",Expr::AppNode,{}},
                    const smt::Term & src_t=smt::Term{},
                    const smt::Term & step=nullptr);

/**
 * Recursively construct a CVC term from an expression
 *
 * @param solver
 * @param astSort - AST datatype from create_datatypes()
 * @param root - term which we will extract a subterm
 * @param pth - Instructions to traverse the AST, e.g. take the 4th arg, then 3rd arg, etc.
 * @return - CVC4 term representing the subterm.
 */
smt::Term constructRec(const smt::SmtSolver & slv,
                       const smt::Sort & astSort,
                       const Expr & tar,
                       const Vi & currpth,
                       const smt::Term & src_t,
                       const std::map<size_t,Vi> & srchsh,
                       const std::map<Vi,size_t> & tarhsh,
                       const smt::Term & step,
                       const std::map<std::string,int> fv,
                       const std::map<std::string,int> & syms);

Expr parseCVCast(const Theory & t, std::shared_ptr<peg::Ast> ast) ;

Expr parseCVC(const Theory & t, const std::string s);
#endif