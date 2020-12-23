#include "../external/catch.hpp"
#include "../src/astextra.hpp"
#include "../src/theories/theories.hpp"

TEST_CASE("patfun")
{
    // Initialize solver and theory
    smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);

    slv->set_opt("produce-models", "true");
    Theory t = cat().upgrade();

    smt::Sort astSort;
    std::tie(astSort, std::ignore, std::ignore) = create_datatypes(slv, t, 2);

    // id.f pattern matches with id.f
    Expr expr = t.rules.at(0).t2;
    smt::Term x = construct(slv, astSort, t, expr);
    smt::Term p = pat_fun(slv, t, x, 1, "r");
    slv->assert_formula(p);
    CHECK(slv->check_sat().is_sat());

    // f doesn't pattern match with id.f
    Expr expr2 = t.rules.at(0).t1;
    smt::Term x2 = construct(slv, astSort, t, expr2);
    smt::Term p2 = slv->make_term(smt::Not, pat_fun(slv, t, x2, 1, "r"));
    slv->assert_formula(p2);
    CHECK(slv->check_sat().is_sat());

    // id.(f.g.h) pattern matches with id.f
    Expr fgh = t.rules.at(2).t1.uninfer();
    Expr ida = expr.args.at(1).uninfer();
    Expr expr3 = t.upgrade(App("cmp", {ida, fgh}));
    smt::Term x3 = mkConst(slv, "x3", construct(slv, astSort, t, expr3));
    smt::Term p3 = pat_fun(slv, t, x3, 1, "r");
    slv->assert_formula(p3);
    CHECK(slv->check_sat().is_sat());

    writeModel(slv, "test/patfun.dat");
}

TEST_CASE("rterm_fun")
{
    // Initialize solver and theory
    smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);
    slv->set_opt("produce-models", "true");
    Theory t = cat().upgrade();

    smt::Sort astSort;
    std::tie(astSort, std::ignore, std::ignore) = create_datatypes(slv, t, 2);

    // Initial and final terms of a rule
    Expr f = t.rules.at(0).t1, idf = t.rules.at(0).t2;
    // SMT-lib term rule is applied to
    smt::Term f1 = construct(slv, astSort, t, f);
    // Computed result of applying the rule
    smt::Term idf2 = mkConst(slv, "idf2", rterm_fun(slv, t, f1, 0, 1, "f"));
    // Make sure it's equal to the final term of the rule
    CHECK(check_equal(slv, astSort, t, idf2, idf));

    mkConst(slv, "idf1", construct(slv, astSort, t, idf)); // for reference

    writeModel(slv, "test/rtermfun.dat");
}

TEST_CASE("getAt")
{
    // Set up solver and theory
    smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);
    slv->set_opt("produce-models", "true");
    Theory t = cat().upgrade();
    Vvvi paths = all_paths(2, 2);
    smt::Sort astSort, pathSort;
    std::tie(astSort, pathSort, std::ignore) = create_datatypes(slv, t, 2);

    // Create example terms from the theory of categories
    Expr f_gh = t.rules.at(2).t1;
    Expr f = f_gh.args.at(1);
    Expr g = f_gh.args.at(2).args.at(1);

    // A complex SMT-lib AST term with substructure.
    smt::Term f_gh1 = construct(slv, astSort, t, f_gh);

    // Some paths to use with getAt
    smt::Term pEmpty = unit(slv, pathSort, "Empty");
    smt::Term p1 = unit(slv, pathSort, "P1");
    smt::Term p21 = unit(slv, pathSort, "P21");
    smt::Term p22 = unit(slv, pathSort, "P22");

    // Subterms from getAt
    smt::Term f_gh2 = mkConst(slv, "f_gh", getAt(slv, f_gh1, pEmpty, paths));
    smt::Term f2 = getAt(slv, f_gh1, p1, paths);
    smt::Term g2 = getAt(slv, f_gh1, p21, paths);
    smt::Term h2 = getAt(slv, f_gh1, p22, paths);

    //Test whether getAt gives expected results
    CHECK(check_equal(slv, astSort, t, f_gh2, f_gh));
    CHECK(check_equal(slv, astSort, t, f2, f));
    CHECK(check_equal(slv, astSort, t, g2, g));
    CHECK_FALSE(check_equal(slv, astSort, t, h2, g));

    writeModel(slv, "test/getat.dat");
}

TEST_CASE("replaceAt")
{
    // Set up solver and theory
    smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);

    slv->set_opt("produce-models", "true");
    Theory t = monoid().upgrade();
    Vvvi paths = all_paths(2, 2);
    smt::Sort astSort, pathSort;
    std::tie(astSort, pathSort, std::ignore) = create_datatypes(slv, t, 2);

    // Create expressions for testing
    Expr x_yz = t.rules.at(2).t1, x = x_yz.args.at(1);
    Expr u_xyz = x_yz.uninfer();
    Expr ux = x.uninfer(), uy = u_xyz.args.at(1).args.at(0);
    Expr x_yx = t.upgrade(App("M", {ux, App("M", {uy, ux})}));

    // Create SMT-lib terms for testing
    smt::Term xyz_ = mkConst(slv, "xyz", construct(slv, astSort, t, x_yz));
    smt::Term xyx_ = mkConst(slv, "xyx", construct(slv, astSort, t, x_yx));
    smt::Term x_ = mkConst(slv, "x", construct(slv, astSort, t, x));
    smt::Term p22 = unit(slv, pathSort, "P22");
    smt::Term pEmpty = unit(slv, pathSort, "Empty");
    smt::Term result = mkConst(slv, "result", replaceAt(slv, xyz_, x_, p22, paths));
    smt::Term x2 = replaceAt(slv, xyz_, x_, pEmpty, paths);

    CHECK(check_equal(slv, astSort, t, result, x_yx));
    CHECK(check_equal(slv, astSort, t, x2, x));

    writeModel(slv, "test/replaceat.dat");
}

TEST_CASE("rewriteTop")
{
}

TEST_CASE("assert_rewrite")
{
}