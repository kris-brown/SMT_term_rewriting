#include "catch.hpp"
#include "../src/theories/cat.hpp"
#include "../src/theories/intarray.hpp"
#include "../src/theories/boolalg.hpp"

TEST_CASE("cat") {
    CHECK_NOTHROW(upgradeT(cat()));
    CHECK_NOTHROW(upgradeT(intarray()));
    CHECK_NOTHROW(upgradeT(boolalg()));
}