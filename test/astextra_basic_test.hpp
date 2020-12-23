#include "../external/catch.hpp"
#include "../src/astextra_basic.hpp"
#include "../src/astextra.hpp"
#include "../src/cvc4extra.hpp"
#include "../src/theories/theories.hpp"

TEST_CASE("cart_product")
{
    // The single element lists are irrelevant to the size of cartesian product
    // So we expect result size to be 2 x 3 x 2 = 12
    Vvi input{{1, 2}, {1, 2, 3}, {1}, {1}, {1, 2}};
    Vi tmp;
    Vvi output;
    cart_product(output, tmp, input.begin(), input.end());
    CHECK(output.size() == 12);
}

TEST_CASE("paths_n")
{
    // All paths of depth 3 with values 0/1/2
    CHECK(paths_n(3, 2).size() == 27);
}

TEST_CASE("all_paths")
{
    // All paths of depth 2 with values 0/1/2
    CHECK(all_paths(3, 2).at(1).size() == 9);
}

TEST_CASE("replace1")
{
    // Initialize solver and theory
    smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);
    slv->set_opt("produce-models", "true");

    Theory t = natarray();
    smt::Sort astSort, Int = slv->make_sort(smt::INT);
    std::tie(astSort, std::ignore, std::ignore) = create_datatypes(slv, t, 2);
    smt::Datatype dt = astSort->get_datatype();

    // Max args is 3 due to `write` function, + 1 for sort information
    CHECK(arity(astSort) == 4);

    // Create some terms to test on
    smt::Term seven = slv->make_term(7, Int);
    smt::Term l7 = ast(slv, astSort, seven);
    smt::Term l8 = ast(slv, astSort, slv->make_term(8, Int));
    smt::Term l17 = mkConst(slv, "l17", ast(slv, astSort, slv->make_term(1, Int), {l7}));
    smt::Term l18 = mkConst(slv, "l18", ast(slv, astSort, slv->make_term(1, Int), {l8}));
    smt::Term subl18 = mkConst(slv, "l178", replace(slv, 0, l17, l8));

    // Check access of node of an AST term
    slv->assert_formula(slv->make_term(smt::Equal, node(slv, l7), seven));
    CHECK(slv->check_sat().is_sat());

    // Simple substitution of first argument of (1,{l7}) with l8 to get (1, {l8})
    slv->assert_formula(slv->make_term(smt::Equal, l18, subl18));
    CHECK(slv->check_sat().is_sat());

    // Log output
    writeModel(slv, "test/replacet.dat");
}

TEST_CASE("construct")
{
    // Initialize solver and theory
    smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);
    slv->set_opt("produce-models", "true");

    Theory t = monoid();
    smt::Sort astSort;
    std::tie(astSort, std::ignore, std::ignore) = create_datatypes(slv, t, 2);
    smt::Sort Int = slv->make_sort(smt::INT);

    // Mapping from symbols to the nodes in SMT-lib AST terms
    std::map<std::string, int> sc = t.symcode();

    // Create SMT-lib terms manually
    smt::Term ob1 = mkConst(slv, "Ob", ast(slv, astSort, slv->make_term(sc["Ob"], Int)));
    smt::Term x1 = mkConst(slv, "x", ast(slv, astSort, slv->make_term(sc["x"], Int), {ob1}));
    smt::Term y1 = mkConst(slv, "y", ast(slv, astSort, slv->make_term(sc["y"], Int), {ob1}));
    smt::Term q1 = mkConst(slv, "q", ast(slv, astSort, slv->make_term(strhash("q"), Int), {ob1}));

    // Create expressions for testing
    Expr Ob = Srt("Ob");
    Expr x = Var("x", Ob), y = Var("y", Ob), q = Var("q", Ob);
    Expr xy = App("M", {x, y}), yx = App("M", {y, x});
    Expr xyq = App("M", {x, App("M", {y, q})}), qyx = App("M", {q, yx}), xyx = App("M", {x, yx});

    // Construct SMT-lib terms from construct()
    smt::Term x2 = mkConst(slv, "x2", construct(slv, astSort, t, x));
    smt::Term q2 = mkConst(slv, "q2", construct(slv, astSort, t, q));

    // Test constructions with empty context
    CHECK(check_equal(slv, astSort, t, x1, x)); // "x" is already in theory
    CHECK(check_equal(slv, astSort, t, q1, q)); // "q" was not in theory, so strhash was used for q1

    smt::Term xy1 = mkConst(slv, "xy", construct(slv, astSort, t, xy));
    smt::Term xyx1 = mkConst(slv, "xyx", construct(slv, astSort, t, xyx, xy, xy1));

    // Test constructions are the same with and without context
    CHECK(check_equal(slv, astSort, t, xyx1, xyx));

    writeModel(slv, "test/construct.dat"); // Log output model
}

TEST_CASE("replPfun")
{
    // Initialize solver and theory
    smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);
    slv->set_opt("produce-models", "true");
    Theory t = monoid().upgrade();
    Vvvi paths = all_paths(2, 2);
    smt::Sort astSort, pathSort;
    std::tie(astSort, pathSort, std::ignore) = create_datatypes(slv, t, 2);

    // Create expressions for testing
    Expr Ob = Srt("Ob"), x = Var("x", Ob), y = Var("y", Ob);
    Expr xy = App("M", {x, y}), xx = App("M", {x, x});

    // Replace the second argument of "(x:Ob⋅y:Ob)" with x:Ob to get "(x:Ob⋅x:Ob)"
    smt::Term xy_ = mkConst(slv, "xy", construct(slv, astSort, t, t.upgrade(xy)));
    smt::Term x_ = mkConst(slv, "x", construct(slv, astSort, t, t.upgrade(x)));

    smt::Term res = replP_fun(slv, xy_, x_, {2});

    smt::Term result = mkConst(slv, "result", res);

    writeModel(slv, "test/replP.dat");
    CHECK(check_equal(slv, astSort, t, result, t.upgrade(xx)));
}

TEST_CASE("subterm")
{
}

TEST_CASE("parseCVC")
{
    // Initialize solver and theory
    smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);

    slv->set_opt("produce-models", "true");
    Theory t = cat().upgrade();
    smt::Sort astSort;
    std::tie(astSort, std::ignore, std::ignore) = create_datatypes(slv, t, 2);

    // Check if construct -> parseCVC = identity
    Expr expr = t.rules.at(0).t2;
    smt::Term x = mkConst(slv, "x", construct(slv, astSort, t, expr));
    slv->check_sat();
    std::string to_parse = slv->get_value(x)->to_string();
    Expr expr_ = parseCVC(t, to_parse);
    CHECK((expr == expr_));

    // Now something with variables that are not in the theory
    Expr o = Srt("Ob"), q = Var("Q", {o}), z = Var("Z", {o});
    Expr h = Srt("Hom", {q, z}), m = Var("m", {h});
    smt::Term m_ = mkConst(slv, "m", construct(slv, astSort, t, m));
    slv->check_sat(); // required to make model so get_value can work
    Expr x2_ = parseCVC(t, slv->get_value(m_)->to_string());
    CHECK((m == x2_));

    writeModel(slv, "test/parsecvc.dat");
}
