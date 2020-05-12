#include "catch.hpp"
#include "../src/theories/theories.hpp"

// Needed to test running main program on theories
#include "../src/cvc4extra.hpp"
#include "../src/astextra.hpp"

TEST_CASE("cat") {
    for (auto && t: alltheories()) {
        CHECK_NOTHROW(upgradeT(t));
        CVC::Solver slv;
        slv.setOption("produce-models", "true");
        setup(slv,t,2);
        writeModel(slv,"build/"+t.name+".dat");
    }


}