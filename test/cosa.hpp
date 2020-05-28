#include "../external/catch.hpp"


/*
* Getting the hang of making a transition system with Cosa2 using SMT_switch C++ api
*/
#include "smt-switch/smt.h"
#include "smt-switch/cvc4_factory.h"


#include "../../cosa2clone/core/fts.h"
#include "../../cosa2clone/core/prop.h"

TEST_CASE("transition system") {
   smt::SmtSolver s=smt::CVC4SolverFactory().create(false);

   cosa::FunctionalTransitionSystem fts(s);
   CHECK(fts.is_functional());

   smt::Term term = s->make_term(5, s->make_sort(smt::INT));
   cosa::Property p{fts,term};

}