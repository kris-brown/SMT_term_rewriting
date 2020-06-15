#include "../external/catch.hpp"
#include "../src/theories/theories.hpp"

// Needed to test running main program on theories
#include "../src/cvc4extra.hpp"
#include "../src/astextra.hpp"

TEST_CASE("create_datatypes") {
    for (auto && t: alltheories()) {
        CHECK_NOTHROW(t.upgrade());
        smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);
        slv->set_opt("produce-models", "true");
        create_datatypes(slv,t,2);
        writeModel(slv,"build/"+t.name+".dat");
    }
}

TEST_CASE("parse") {
    for (auto && t: alltheories()) {
        CHECK_NOTHROW(Theory::parseTheory("data/"+t.name+".dat"));
    }
}


