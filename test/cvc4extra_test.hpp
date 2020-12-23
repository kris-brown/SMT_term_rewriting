#include "../external/catch.hpp"
#include "../src/cvc4extra.hpp"

TEST_CASE("mkConst")
{
    smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);

    smt::Sort Int = slv->make_sort(smt::INT);
    CHECK_NOTHROW(mkConst(slv, "x", slv->make_term(9, Int)));
}

TEST_CASE("ITE")
{
    // Initialize solver
    smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);
    smt::Sort Int = slv->make_sort(smt::INT);

    // Create system for: if x > 9 then 100 elif x < 0 then -100 else 7
    Vt thens{slv->make_term(100, Int), slv->make_term(-100, Int)};
    smt::Term Else = slv->make_term(7, Int);

    // Test cases, inputs and expected results
    std::vector<int> args{-1, 10, 5}, res{-100, 100, 7};

    // Run the test cases
    for (int i = 0; i != 3; i++)
    {
        // Create input
        smt::Term x = slv->make_term(args.at(i), Int);
        // Create ITE term
        Vt ifs{slv->make_term(smt::Gt, x, slv->make_term(9, Int)),
               slv->make_term(smt::Lt, x, slv->make_term(0, Int))};
        smt::Term t = ITE(slv, ifs, thens, Else);

        // Assert result is expected
        slv->assert_formula(slv->make_term(smt::Equal, t,
                                           slv->make_term(res.at(i), Int)));

        CHECK(slv->check_sat().is_sat());
    }

    // ifs and thens don't have same size
    CHECK_THROWS(ITE(slv, {Else, Else}, {Else}, Else));
}
