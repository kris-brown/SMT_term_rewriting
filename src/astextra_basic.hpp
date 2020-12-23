#ifndef ASTEXTRABASIC
#define ASTEXTRABASIC
#include <vector>
#include "smt-switch/smt.h"
#include "theory.hpp"

typedef std::vector<int> Vi;
typedef std::vector<Vi> Vvi;
typedef std::vector<Vvi> Vvvi;
typedef std::vector<smt::Term> Vt;

/**
 * Cartesian product of vector of vectors
 * taken from: https://stackoverflow.com/a/5279601
 *
 * @param rvvi final result (gets updated)
 * @param rvi temporary storage (current result)
 * @param me input vector begin
 * @param end input vector end
 */
void cart_product(
    Vvi &rvvi,
    Vi &rvi,
    Vvi::const_iterator me,
    Vvi::const_iterator end);

/**
 * Path vector generation: we want (arity ^ depth) number of paths
 *
 * @param depth final result (gets updated)
 * @param arity number of options to take at each step in path
 * @return vector of paths, e.g. [[0,0],[0,1],[1,0],[1,1]] for arity=2, depth=2
 */
Vvi paths_n(const int &depth, const int &arity);

/**
 * All paths of length 1, 2, ... depth
 *
 * @param depth the largest path length, which we will build up to
 * @param arity number of options to take at each step in path
 * @return vector of vector of paths
 */
Vvvi all_paths(const int &depth, const int &arity);

/**
 * Create instance of some constructor which takes no arguments
 *
 * @param slv Solver
 * @param srt Inductive datatype sort
 * @param name Name of the constructor
 * @return a SMT-switch term (e.g. None, Empty, Error)
 */
smt::Term unit(const smt::SmtSolver &slv,
               const smt::Sort &srt,
               const std::string &name);
/**
 * Number of AST selectors for the ast constructor
 *
 * @param astSort AST sort from create_datatypes()
 * @return the highest i for which there is a selector a_i::AST
 */
int arity(const smt::Sort &astSort);

/**
 * Apply node selector to an AST term (which has a node and args).
 *
 * @param slv - Solver
 * @param x - input term (must be AST sort)
 * @return - a CVC term with the node of the input
 */
smt::Term node(const smt::SmtSolver &slv,
               const smt::Term &x);

/**
 * Apply arg selector to an AST term.
 *
 * @param solver
 * @param x input term (must be AST sort)
 * @param i which argument we want to select
 * @return a CVC term with the node of the input
 */

smt::Term getarg(const smt::SmtSolver &slv,
                 const smt::Term &x,
                 const int &i);
/**
 * Construct a term with AST's ast constructor.
 *
 * @param solver
 * @param astDT AST datatype from create_datatypes()
 * @param n Value of the node (must be INT sort)
 * @param xs arguments of the node (all must be AST sort)
 * @return the three sorts
 */
smt::Term ast(const smt::SmtSolver &slv,
              const smt::Sort &astSort,
              const smt::Term &n,
              const Vt &xs = {});

/**
 * Replace a top-level argument of an AST.
 *
 * @param solver
 * @param astDT AST datatype from create_datatypes()
 * @param arg Which argument to replace
 * @param x term with an arg to be replaced (must be nonError AST sort)
 * @param y term substituted for the arg (must be AST sort)
 * @return the modified SMT-lib AST term
 */

smt::Term replace(const smt::SmtSolver &slv,
                  const int &arg,
                  const smt::Term &x,
                  const smt::Term &y);

/**
 * Take first n elements of a vector (return a copy)
 *
 * @param vec Vector of integers
 * @param n Number of elements (error if greater than vec's size)
 * @return Copy of truncated vector.
 */
Vi take(const Vi &vec, const size_t &n);

/**
 * Replace an arbitrary subterm, specified by a path
 *
 * @param solver
 * @param x term for which we will replace a subterm (must be AST sort)
 * @param y replacement term (must be AST sort)
 * @param pth Location of subterm
 * @return SMT-lib AST term representing the substitution result.
 */
smt::Term replP_fun(const smt::SmtSolver &slv,
                    const smt::Term &x,
                    const smt::Term &y,
                    const Vi &pth);
/**
 * Apply selectors a1(a3(a4(...(x)))), specified by a vector
 *
 * @param solver
 * @param astDT AST datatype from create_datatypes()
 * @param root term which we will extract a subterm
 * @param pth Instructions to traverse the AST, e.g. take the 4th arg, then 3rd arg, etc.
 * @return SMT-lib AST term representing the subterm.
 */
smt::Term subterm(const smt::SmtSolver &slv,
                  const smt::Term &root,
                  const Vi &pth);

/**
 * Create arbitrary positive integer for a variable name.
 *
 * @param s The input string to be hashed
 * @return an int between 100 and 10000.
 */
int64_t strhash(const std::string &s);

/**
 * Invert strhash()
 *
 * @param s an int between 100 and 10000.
 * @return A variable name.
 */
std::string strhashinv(const int64_t &i);

/**
 * Predicate which applies a tester to a term
 *
 * @param solver
 * @param x a term (AST/Rule/Path)
 * @param s a constructor name (e.g. Empty/Error/P132)
 * @returns A SMT-lib term which evaluates to a bool
*/
smt::Term test(const smt::SmtSolver &slv,
               const smt::Term &x,
               const std::string &s);

/**
 * Negation of test(slv, x, s)
 *
 * @param solver
 * @param x a term (AST/Rule/Path)
 * @param s a constructor name (e.g. Empty/Error/P132)
 * @returns A SMT-lib term which evaluates to a bool
*/
smt::Term ntest(const smt::SmtSolver &slv,
                const smt::Term &x,
                const std::string &s);

/**
 * Build a term in reference to another term,
 * e.g. y from x
 * x=(1+(2+3)) and y=((2+3)+2)
 * then we get y=(x.1 + x.1.0)
 *
 * If no source/reference term is given, it just constructs
 * the natural SMT-switch term corresponding to the `Expr`
 *
 * @param solver
 * @param astSort AST sort from create_datatypes()
 * @param tar term which we will construct with SMT-lib api
 * @param src optional term which we can reference in our construction
 * @param step Seed to produce distinct free variables, if any
 * @return SMT-lib AST term representing tar.
 */
smt::Term construct(const smt::SmtSolver &slv,
                    const smt::Sort &astSort,
                    const Theory &t,
                    const Expr &tar,
                    const Expr &src = Expr{"?", Expr::AppNode, {}},
                    const smt::Term &src_t = smt::Term{},
                    const smt::Term &step = nullptr);

/**
 * Recursively construct a CVC term from an expression
 *
 * @param solver
 * @param astSort AST datatype from create_datatypes()
 * @param tar term which we will construct with SMT-lib api
 * @param currpth location within the target term we are working on
 * @param src_t
 * @param srchsh
 * @param tarhsh
 * @param step
 * @param fv free variables
 * @param syms mapping of symbols to their INT-encoded values
 * @return SMT-lib AST term representing tar.
 */
smt::Term constructRec(const smt::SmtSolver &slv,
                       const smt::Sort &astSort,
                       const Expr &tar,
                       const Vi &currpth,
                       const smt::Term &src_t,
                       const std::map<size_t, Vi> &srchsh,
                       const std::map<Vi, size_t> &tarhsh,
                       const smt::Term &step,
                       const std::map<std::string, int> fv,
                       const std::map<std::string, int> &syms);

/**
 * Helper function for parseCVC
 *
 * @param t Theory by which to interpret the parsed SMT-lib AST term
 * @param ast a PEGlib AST parse result
 * @returns A SMT-lib term which evaluates to a bool
*/
Expr parseCVCast(const Theory &t, std::shared_ptr<peg::Ast> ast);

/**
 * Negation of test(slv, x, s)
 *
 * @param solver
 * @param t Theory by which to interpret the parsed SMT-lib AST term
 * @param s a raw string produced by SMT-lib's representing of an AST term
 * @returns An expression corresponding to the term
*/
Expr parseCVC(const Theory &t, const std::string s);

#endif