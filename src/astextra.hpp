#ifndef ASTEXTRA
#define ASTEXTRA
#include<vector>
#include <cvc4/api/cvc4cpp.h>
#include "astextra_basic.hpp"

typedef std::map<int, CVC::Term> Tmap;
typedef std::vector<Rule> Vr;



/**
 * Create AST, Rule, Path sorts
 *
 * @param solver
 * @param t - Theory dictates the arity of AST and the # of rules
 * @param depth - Arity + depth determines the possible paths
 * @return - the three sorts
 */
std::tuple<CVC::Sort,CVC::Sort,CVC::Sort> create_datatypes(
    CVC::Solver & slv,
    const Theory & t,
    const int & depth);

/**
 * Evaluate whether a term satisfies the input pattern of a rewrite rule.
 *
 * @param slv - solver
 * @param thry - A theory with rewrite rulse
 * @param x - A term we are testing
 * @param r - Index to which rule we are talking about
 * @param dir - Forward or reverse direction?
 * @return A CVC term which evalutes to a bool
 */
CVC::Term pat_fun(const CVC::Solver & slv,
                  const Theory & thry,
                  const CVC::Term & x,
                  const int & r,
                  const char & dir);

/**
 * Construct the resulting term of a rewrite in the context of a term which matches the input pattern.
 *
 * @param slv - solver
 * @param thry - A theory with rewrite rulse
 * @param x - A term which matches the input pattern
 * @param step - which rewrite step we are on (needed to make variables introduced distinct)
 * @param ruleind - Index to which rule we are talking about
 * @param dir - Forward or reverse direction?
 * @return A CVC term which matches the result pattern
 */
CVC::Term rterm_fun(const CVC::Solver & slv,
                    const Theory & thry,
                    const CVC::Term & x,
                    const int & step,
                    const int & ruleind,
                    const char & dir);

/**
 * Access a subterm via a path CVC term.
 *
 * @param solver
 * @param xTerm - term from which we wish to look at a subterm
 * @param pTerm - CVC4 term which refers to some path
 * @param paths - All possible paths
 * @return a CVC4 subterm of xTerm
 */

CVC::Term getAt(const CVC::Solver & slv,
                const CVC::Term & xTerm,
                const CVC::Term & pTerm,
                const Vvvi & paths);
/**
 * Access a subterm via a path CVC term.
 *
 * @param solver
 * @param xTerm - term from which we wish to substitute in
 * @param yTerm - term to insert into xTerm
 * @param pTerm - CVC4 term which refers to some path to the location of insertino
 * @param paths - All possible paths
 * @return Result of subtitution.
 */

CVC::Term replaceAt(const CVC::Solver & slv,
                    const CVC::Term & xTerm,
                    const CVC::Term & yTerm,
                    const CVC::Term & pTerm,
                    const Vvvi & paths);

/**
 * Apply rewrite rule to a top-level term
 *
 * @param solver
 * @param x - Term we are rewriting
 * @param rTerm - a CVC term of sort Rule
 * @param t - Theory of which x is a term
 * @param step - Which rewrite step we are on (needed to make variables introduced distinct)
 * @param returns - Either Error or the substitution result
*/
CVC::Term rewriteTop(const CVC::Solver & slv,
                    const CVC::Term & x,
                    const CVC::Term & rTerm,
                    const Theory & t,
                    const int & step);
/**
 * ASSERT that t1 can be rewritten into t2 in (exactly) some number of rewrites.
 *
 * @param solver
 * @param x - incoming term for this rewrite step
 * @param r - variable for the rule applied
 * @param p - variable for the subterm rule is applied to
 * @param steps - Which rewrite step we are on
 */

CVC::Term rewrite(const CVC::Solver & slv,
                  const Theory & t,
                  const CVC::Term & x,
                  const CVC::Term & r,
                  const CVC::Term & p,
                  const int & step,
                  const int & depth
                  ) ;
/**
 * ASSERT that t1 can be rewritten into t2 in (exactly) some number of rewrites.
 *
 * @param solver
 * @param astSort - AST datatype from create_datatypes()
 * @param t - Theory which t1,t2 are terms of
 * @param t1 - starting point
 * @param t2 - destination
 * @param steps - number of rewrites expected
 */

CVC::Term assert_rewrite(const CVC::Solver & slv,
             const CVC::Sort & astSort,
             const CVC::Sort & pathSort,
             const CVC::Sort & ruleSort,
             const Theory & t,
             const CVC::Term & t1,
             const CVC::Term & t2,
             const int & steps
             );
#endif