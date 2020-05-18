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


TEST_CASE("infer") {

    Theory t=cat();
    Theory ut=upgradeT(t);
    // infer that id.f  is Hom(A,B)
    Expr idf = ut.rules.at(0).t2;
    Expr f=idf.args.at(2), homab = f.args.at(0);
    Expr homab2=infer(ut.sorts,ut.ops,idf.sym,
                      {idf.args.at(1),idf.args.at(2)});

    CHECK(homab==homab2);
    // Composing g.f throws an error
    Expr g=ut.rules.at(2).t2.args.at(1).args.at(2);
    CHECK_THROWS(infer(ut.sorts,ut.ops,"cmp",{g,f}));

}
TEST_CASE("expr parser and printer") {
    Theory t=natarray();
    Vx xs = parse_exprs(t,"data/arrayterms.dat");
    for (auto x: xs){
        CHECK(parse_expr(t, print(t,x))==x);}

    Theory h = cat();
    Expr fg = parse_expr(h,"(f:(A:Ob⇒B:Ob) ⋅g:(B:Ob ⇒ C:Ob))");

    CHECK(fg==parse_expr(h,print(h,fg)));
}



TEST_CASE("parse theory") {
    Theory t1 = parseTheory("data/cat.dat");
    Theory t2 = cat();
    CHECK(print(t1)==print(t2));
    CHECK(t1.sorts==t2.sorts);
    CHECK(t1.ops==t2.ops);
    CHECK(t1.rules==t2.rules);
    CHECK(t1==t2);

}

TEST_CASE("mk_freevar") {
    Expr s=Sort("X");
    Expr x=Var("x",s),y=Var("y",s),z=Var("z",s);
    Expr xy=App("+",{x,y}),xyz=App("f",{z,y,x});
    CHECK(freevar(xy,xyz).empty());
    std::map<std::string,int>m{{"z",1}};
    CHECK(freevar(xyz,xy) == m);
}
