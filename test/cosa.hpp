#include "../external/catch.hpp"


/*
* Getting the hang of making a transition system with Cosa2 using SMT_switch C++ api
*/
#include "smt-switch/smt.h"
#include "smt-switch/cvc4_factory.h"


// These are members of an ENUM in COSA2 but macros in mac os SDK
#undef FALSE
#undef TRUE
#include "core/fts.h"
#include "engines/bmc.h"
#include "utils/logger.h"
#include "utils/logger.h"

TEST_CASE("transition system") {

  smt::SmtSolver s = smt::CVC4SolverFactory::create(false);
  s->set_opt("produce-models", "true");
  s->set_opt("incremental", "true");
  cosa::FunctionalTransitionSystem fts(s);

  // simple example with a counter
  smt::Sort bvsort4 = s->make_sort(smt::BV, 4);
  smt::Term cnt = fts.make_state("cnt", bvsort4);

  // initial state has cnt = 0
  fts.constrain_init(s->make_term(smt::Equal, cnt, s->make_term(0, bvsort4)));

  // update function for cnt: cnt' = cnt < 5 ? cnt + 1 : 0
  fts.assign_next(cnt,
                  s->make_term(smt::Ite,
                               s->make_term(smt::BVUlt, cnt, s->make_term(5, bvsort4)),
                               s->make_term(smt::BVAdd, cnt, s->make_term(1, bvsort4)),
                               s->make_term(0, bvsort4)));

  // false property
  smt::Term p = s->make_term(smt::BVUlt, cnt, s->make_term(6, bvsort4));

  cosa::Property prop(fts, p);

  // Note: currently can only use a solver once
  // e.g. couldn't use in a different engine after this -- would need a new solver
  // or it might be slow because old assertions will still be there
  // --> not all solvers support reset_assertions
  cosa::Bmc bmc(prop, s);

  cosa::ProverResult pr = bmc.check_until(5);
  CHECK(pr == cosa::FALSE);


}