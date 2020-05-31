#ifndef CVC4EXTRA
#define CVC4EXTRA
#include<vector>
#include "smt-switch/smt.h"

typedef std::vector<smt::Term> Vt;

/*
 * Helper functions related exclusively to CVC4
 */


//Declare a constant
smt::Term mkConst(const smt::SmtSolver & slv,
                  const std::string & name,
                  const smt::Term & t);


//Chain a series of IF-THEN pairs with an ELSE condition into one term.
smt::Term ITE(const smt::SmtSolver & slv,
                    const Vt & ifs,
                    const Vt & thens,
                    const smt::Term & otherwise);

// Print model (if it is sat) to a path
void writeModel(smt::SmtSolver & slv, std::string pth);

#endif


