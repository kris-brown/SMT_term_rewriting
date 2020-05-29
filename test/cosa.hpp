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


TEST_CASE("transition2") {
  smt::SmtSolver s = smt::CVC4SolverFactory::create(false);

  s->set_opt("produce-models", "true");
  s->set_opt("incremental", "true");
  cosa::FunctionalTransitionSystem fts(s);

  smt::Sort Int = s->make_sort(smt::INT);
  smt::Term state = fts.make_state("cnt", Int);
  smt::Term inp0 = fts.make_input("i0", Int);
  smt::Term inp1 = fts.make_input("i1", Int);
  smt::Term zero = s->make_term(0, Int);
  smt::Term one = s->make_term(1, Int);
  smt::Term two = s->make_term(2, Int);
  smt::Term n1 = s->make_term(-1, Int);
  smt::Term n3 = s->make_term(-3, Int);

  smt::Term i00 = s->make_term(smt::Equal, inp0, zero);
  smt::Term i01 = s->make_term(smt::Equal, inp0, one);
  smt::Term i11 = s->make_term(smt::Equal, inp1, one);
  smt::Term i1n1 = s->make_term(smt::Equal, inp1, n1);
  smt::Term i0const = s->make_term(smt::Or,i00,i01);
  smt::Term i1const = s->make_term(smt::Or,i11,i1n1);
  smt::Term i0i1 = s->make_term(smt::Mult, inp0, inp1);
  smt::Term state1 = s->make_term(smt::Plus, state, one);

  fts.constrain_init(s->make_term(smt::Equal, state, zero));
  fts.constrain_inputs(s->make_term(smt::And,i0const,i1const));
  fts.assign_next(state, s->make_term(smt::Mult, state1, i0i1));
  smt::Term p = s->make_term(
    smt::Not, s->make_term(smt::Equal,state, n3));
  cosa::Property prop(fts, p);
  cosa::Bmc bmc(prop, s);
  cosa::ProverResult pr = bmc.check_until(5);
  CHECK(pr == cosa::FALSE);

  std::vector<smt::UnorderedTermMap> wit;
  bmc.witness(wit);

  std::vector<smt::UnorderedTermMap> expected{
    {{state, zero},{inp1,one},{inp0,one}},
    {{state, one},{inp1,one},{inp0,one}},
    {{state, two},{inp1,n1},{inp0,one}},
    {{state, n3},{inp1,zero},{inp0,zero}}};

  CHECK(wit == expected);
}

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
  smt::Term p = s->make_term(smt::BVUlt, cnt, s->make_term(5, bvsort4));

  cosa::Property prop(fts, p);
  cosa::Bmc bmc(prop, s);

  cosa::ProverResult pr = bmc.check_until(5);
  CHECK(pr == cosa::FALSE);

}

// TEST_CASE("transition_with_datatype") {
//   smt::SmtSolver s = smt::CVC4SolverFactory::create(false);

//   Theory t=upgradeT(monoid());
//   Vvvi paths = all_paths(2,2);
//   CVC::Sort astSort, pathSort, ruleSort;
//   std::tie (astSort,pathSort,ruleSort) = create_datatypes(s,t,2);

//   s->set_opt("produce-models", "true");
//   s->set_opt("incremental", "true");
//   cosa::FunctionalTransitionSystem fts(s);
// }
