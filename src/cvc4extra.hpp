#ifndef CVC4EXTRA
#define CVC4EXTRA
#include<vector>
#include <cvc4/api/cvc4cpp.h>
typedef std::vector<CVC4::api::Term> Vt;


/*
Declare a constant
*/
CVC4::api::Term mkConst(const CVC4::api::Solver & slv,
                  const std::string & name,
                  const CVC4::api::Term & t);

/*
Chain a series of IF-THEN pairs with an ELSE condition into one term.
*/
CVC4::api::Term ITE(const CVC4::api::Solver & slv,
                    const Vt & ifs,
                    const Vt & thens,
                    const CVC4::api::Term & otherwise);
#endif