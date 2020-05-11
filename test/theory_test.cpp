#include "catch.hpp"
#include "../src/theory.hpp"

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

// AND SO ON