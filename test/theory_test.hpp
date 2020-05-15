#include "../external/catch.hpp"
#include "../src/theory.hpp"
#include "../src/theories/theories.hpp"

TEST_CASE("Expr Constructor") {
    // Symbol is empty
    CHECK_THROWS(Expr{"",VarNode,{}});
    CHECK_NOTHROW(Expr{"X",SortNode,{}});
    Expr x{"X",SortNode,{}};
    Expr y{"Y",AppNode,{}};
    CHECK(x!=y);
    // Sort has sort as child
    CHECK_THROWS(Expr{"X",SortNode,{x,y}});

    // Variable needs exactly one child
    CHECK_THROWS(Expr{"V",VarNode,{x,y}});
    CHECK_THROWS(Expr{"V",VarNode,{}});

    CHECK_NOTHROW(Expr{"X",SortNode,{y,y}});
}


TEST_CASE("SubExpr") {
    // Symbol is empty
    CHECK_THROWS(Expr{"",VarNode,{}});
    CHECK_NOTHROW(Expr{"X",SortNode,{}});
    Expr x{"X",AppNode,{}};
    Expr y{"Y",AppNode,{x,x}};
    CHECK(subexpr(y,{1})==x);
    CHECK_THROWS(subexpr(y,{1,1}));
}

TEST_CASE("Sub") {
    Expr x{"X",AppNode,{}},y{"Y",AppNode,{}};
    Expr fxx=App("f",{x,x}),gy=App("g",{y});
    Expr fgygy=App("f",{gy,gy});
    MatchDict m{{"X",gy}};
    CHECK(sub(fxx,m)==fgygy);
}

TEST_CASE("expr parser and printer") {
    Theory t=natarray();
    Vx xs = parse_exprs(t,"data/arrayterms.dat");
    for (auto x: xs){
        CHECK(parse_expr(t, print(t,x))==x);}
}

TEST_CASE("parse theory") {
    Theory t1 = parseTheory("data/category.dat");
    Theory t2 = cat();
    CHECK(print(t1)==print(t2));
    CHECK(t1.sorts==t2.sorts);
    CHECK(t1.ops==t2.ops);
    CHECK(t1.rules==t2.rules);
    CHECK(t1==t2);

}

