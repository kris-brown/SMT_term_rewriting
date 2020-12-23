#ifndef CVC4EXTRA
#define CVC4EXTRA

/*
 * Helper functions related exclusively to CVC4
 */

#include <vector>
#include "smt-switch/smt.h"

typedef std::vector<smt::Term> Vt;
typedef std::vector<std::pair<std::string, std::string>> ResStep;
typedef std::vector<ResStep> Res;

/**
 * Declare a constant to be equal to a term
 * @param slv Solver - modified to make the assertion
 * @param name Symbol
 * @param t Term
 * @returns A term representing the constant symbol
 */
smt::Term mkConst(const smt::SmtSolver &slv,
                  const std::string &name,
                  const smt::Term &t);

/**
 * Chain a series of IF-THEN pairs with an ELSE condition into one term.
 * @param slv Solver
 * @param ifs A vector of boolean SMT-switch terms
 * @param thens Equal number of corresponding THEN clauses to match the IFs
 * @param otherwise
 * @returns nested if then ... elif then ... else... term
 */
smt::Term ITE(const smt::SmtSolver &slv,
              const Vt &ifs,
              const Vt &thens,
              const smt::Term &otherwise);

/**
 * Print model (if it is sat) to a path
 * @param slv Solver
 * @param pth Path to file to print
 */
void writeModel(smt::SmtSolver &slv, std::string pth);

/**
 * Render the results of a transition system result
 * @param maps result from bounded model checking witness
 * @returns array of sorted pairs representing the terms at each step
 */
Res printResult(std::vector<smt::UnorderedTermMap> maps);

#endif
