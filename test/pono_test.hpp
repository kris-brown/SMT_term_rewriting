#include "../external/catch.hpp"

/*
* Getting the hang of making a transition system with Pono using SMT_switch C++ api
*/
#include "smt-switch/smt.h"
#include "smt-switch/cvc4_factory.h"
#include "../src/cvc4extra.hpp"
#include <string>

// These are members of an ENUM in Pono but macros in mac os SDK
#undef FALSE
#undef TRUE
#include "core/fts.h"
#include "engines/bmc.h"
#include "utils/logger.h"
#include "utils/logger.h"

TEST_CASE("transition system")
{
  // Set up solver and transition system
  smt::SmtSolver s = smt::CVC4SolverFactory::create(false);
  s->set_opt("produce-models", "true");
  s->set_opt("incremental", "true");
  pono::FunctionalTransitionSystem fts(s);

  // simple example with a counter
  smt::Sort bvsort4 = s->make_sort(smt::BV, 4);
  smt::Term cnt = fts.make_statevar("cnt", bvsort4);

  // initial state has cnt = 0
  fts.constrain_init(s->make_term(smt::Equal, cnt, s->make_term(0, bvsort4)));

  // update function for cnt: cnt' = cnt < 5 ? cnt + 1 : 0
  auto update = s->make_term(
      smt::Ite,
      s->make_term(smt::BVUlt, cnt, s->make_term(5, bvsort4)),
      s->make_term(smt::BVAdd, cnt, s->make_term(1, bvsort4)),
      s->make_term(0, bvsort4));
  fts.assign_next(cnt, update);

  // Check for false property: cnt < 5
  smt::Term p = s->make_term(smt::BVUlt, cnt, s->make_term(5, bvsort4));

  pono::Property prop(s, p);
  pono::Bmc bmc(prop, fts, s);

  // Run finite model checking and collect result
  pono::ProverResult pr = bmc.check_until(10);
  CHECK(pr == pono::FALSE);

  std::vector<smt::UnorderedTermMap> wit;
  CHECK(bmc.witness(wit));
  Res witres = printResult(wit);

  // Printing bitvector values
  auto bv = [](int i) {
    return "(_ bv" + std::to_string(i) + " 4)";
  };

  // expected results
  Res expected = {
      {{"cnt", bv(0)}, {"cnt.next", bv(1)}},
      {{"cnt", bv(1)}, {"cnt.next", bv(2)}},
      {{"cnt", bv(2)}, {"cnt.next", bv(3)}},
      {{"cnt", bv(3)}, {"cnt.next", bv(4)}},
      {{"cnt", bv(4)}, {"cnt.next", bv(5)}},
      {{"cnt", bv(5)}, {"cnt.next", bv(0)}},
  };

  CHECK(witres == expected);
}

TEST_CASE("transition2")
{
  // Initialize SMTsolver and transition system
  smt::SmtSolver s = smt::CVC4SolverFactory::create(false);

  s->set_opt("produce-models", "true");
  s->set_opt("incremental", "true");
  pono::FunctionalTransitionSystem fts(s);

  smt::Sort Int = s->make_sort(smt::INT);

  // Create variables for transition system
  smt::Term state = fts.make_statevar("state", Int);
  smt::Term inp0 = fts.make_inputvar("i0", Int);
  smt::Term inp1 = fts.make_inputvar("i1", Int);

  // Create smt-lib terms for various integers
  smt::Term zero = s->make_term(0, Int);
  smt::Term one = s->make_term(1, Int);
  smt::Term two = s->make_term(2, Int);
  smt::Term n1 = s->make_term(-1, Int);
  smt::Term n3 = s->make_term(-3, Int);

  // Create terms
  smt::Term eq0_0 = s->make_term(smt::Equal, inp0, zero);
  smt::Term eq0_1 = s->make_term(smt::Equal, inp0, one);
  smt::Term eq1_1 = s->make_term(smt::Equal, inp1, one);
  smt::Term eq1_n1 = s->make_term(smt::Equal, inp1, n1);
  smt::Term eq0_0__or__1 = s->make_term(smt::Or, eq0_0, eq0_1);
  smt::Term eq1_1__or__n1 = s->make_term(smt::Or, eq1_1, eq1_n1);
  smt::Term times_i0i1 = s->make_term(smt::Mult, inp0, inp1);
  smt::Term increment = s->make_term(smt::Plus, state, one);

  // Set up transition system
  fts.constrain_init(s->make_term(smt::Equal, state, zero));
  fts.constrain_inputs(s->make_term(smt::And, eq0_0__or__1, eq1_1__or__n1));
  fts.assign_next(state, s->make_term(smt::Mult, increment, times_i0i1));
  smt::Term p = s->make_term(
      smt::Not, s->make_term(smt::Equal, state, n3));

  pono::Property prop(s, p);
  pono::Bmc bmc(prop, fts, s);
  pono::ProverResult pr = bmc.check_until(5);

  writeModel(s, "test/transition2.dat");
  CHECK(pr == pono::FALSE);

  std::vector<smt::UnorderedTermMap> wit;
  CHECK(bmc.witness(wit));
  Res witres = printResult(wit);

  // expected results
  Res expected = {
      {{"i0", "1"}, {"i1", "1"}, {"state", "0"}, {"state.next", "1"}},
      {{"i0", "1"}, {"i1", "1"}, {"state", "1"}, {"state.next", "2"}},
      {{"i0", "1"}, {"i1", "(- 1)"}, {"state", "2"}, {"state.next", "(- 3)"}},
      {{"i0", "0"}, {"i1", "0"}, {"state", "(- 3)"}, {"state.next", "0"}},
  };
  CHECK(witres == expected);
}
