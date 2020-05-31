#ifndef ASTEXTRA
#define ASTEXTRA

/*
 * Functions related to rewriting AST terms in swt-switch
 */

#include<vector>
#include "astextra_basic.hpp"
#include "smt-switch/smt.h"
typedef std::map<int, smt::Term> Tmap;
typedef std::vector<Rule> Vr;



/**
 * Create AST, Rule, Path sorts
 *
 * @param solver
 * @param t - Theory dictates the arity of AST and the # of rules
 * @param depth - Arity + depth determines the possible paths
 * @return - the three sorts
 */
std::tuple<smt::Sort,smt::Sort,smt::Sort> create_datatypes(
    smt::SmtSolver & slv,
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
smt::Term pat_fun(const smt::SmtSolver & slv,
                  const Theory & thry,
                  const smt::Term & x,
                  const int & r,
                  const std::string & dir);

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
smt::Term rterm_fun(const smt::SmtSolver & slv,
                    const Theory & thry,
                    const smt::Term & x,
                    const smt::Term & step,
                    const int & ruleind,
                    const std::string & dir);

/**
 * Access a subterm via a path CVC term.
 *
 * @param solver
 * @param xTerm - term from which we wish to look at a subterm
 * @param pTerm - CVC4 term which refers to some path
 * @param paths - All possible paths
 * @return a CVC4 subterm of xTerm
 */

smt::Term getAt(const smt::SmtSolver & slv,
                const smt::Term & xTerm,
                const smt::Term & pTerm,
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

smt::Term replaceAt(const smt::SmtSolver & slv,
                    const smt::Term & xTerm,
                    const smt::Term & yTerm,
                    const smt::Term & pTerm,
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
smt::Term rewriteTop(const smt::SmtSolver & slv,
                    const smt::Term & x,
                    const smt::Term & rTerm,
                    const Theory & t,
                    const smt::Term & step);
/**
 * ASSERT that t1 can be rewritten into t2 in (exactly) some number of rewrites.
 *
 * @param solver
 * @param x - incoming term for this rewrite step
 * @param r - variable for the rule applied
 * @param p - variable for the subterm rule is applied to
 * @param steps - Which rewrite step we are on
 */

smt::Term rewrite(const smt::SmtSolver & slv,
                  const Theory & t,
                  const smt::Term & x,
                  const smt::Term & r,
                  const smt::Term & p,
                  const smt::Term & step,
                  const int & depth
                  ) ;
#endif