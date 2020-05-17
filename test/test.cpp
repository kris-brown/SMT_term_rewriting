#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "../external/catch.hpp"

// Copy other tests into this file
#include "theory_test.hpp"
#include "theories_test.hpp"
#include "astextra_basic_test.hpp"
#include "astextra_test.hpp"
#include "cvc4extra_test.hpp"

//------------------------------------------------------------------------------------------------
// DEMO OF CATCH TESTING
unsigned int Factorial( unsigned int number ) {
    return number <= 1 ? 1 : Factorial(number-1)*number;
}

TEST_CASE( "Factorials are computed", "[factorial]" ) {
    REQUIRE( Factorial(0) == 1 );
    REQUIRE( Factorial(1) == 1 );
    REQUIRE( Factorial(2) == 2 );
}
//------------------------------------------------------------------------------------------------
