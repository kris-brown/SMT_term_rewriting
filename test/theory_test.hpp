#include "../external/catch.hpp"
#include "../src/theory.hpp"
#include "../src/theories/theories.hpp"

TEST_CASE("Expr Constructor") {
    // Symbol is empty
    CHECK_THROWS(Expr{"",Expr::VarNode,{}});
    CHECK_NOTHROW(Expr{"X",Expr::SortNode,{}});
    Expr x{"X",Expr::SortNode,{}};
    Expr y{"Y",Expr::AppNode,{}};

    CHECK((x!=y));

    // Sort has sort as child
    CHECK_THROWS(Expr{"X",Expr::SortNode,{x,y}});

    // Variable needs exactly one child
    CHECK_THROWS(Expr{"V",Expr::VarNode,{x,y}});
    CHECK_THROWS(Expr{"V",Expr::VarNode,{}});

    CHECK_NOTHROW(Expr{"X",Expr::SortNode,{y,y}});
}


TEST_CASE("SubExpr") {
    // Symbol is empty
    CHECK_THROWS(Expr{"",Expr::VarNode,{}});
    CHECK_NOTHROW(Expr{"X",Expr::SortNode,{}});
    Expr x{"X",Expr::AppNode,{}};
    Expr y{"Y",Expr::AppNode,{x,x}};
    CHECK((y.subexpr({1})==x));
    CHECK_THROWS(y.subexpr({1,1}));
}


TEST_CASE("Sub") {
    Expr x{"X",Expr::AppNode,{}},y{"Y",Expr::AppNode,{}};
    Expr fxx=App("f",{x,x}),gy=App("g",{y});
    Expr fgygy=App("f",{gy,gy});
    MatchDict m{{"X",gy}};
    CHECK((fxx.sub(m)==fgygy));
}


TEST_CASE("infer") {

    Theory t=cat();
    Theory ut=t.upgrade();

    // infer that id.f  is Hom(A,B)
    Expr idf = ut.rules.at(0).t2;
    Expr f=idf.args.at(2), homab = f.args.at(0);
    Expr homab2=Expr::infer(ut.sorts,ut.ops,idf.sym,
                      {idf.args.at(1),idf.args.at(2)});

    CHECK(homab==homab2);
    // Composing g.f throws an error
    Expr g=ut.rules.at(2).t2.args.at(1).args.at(2);
    CHECK_THROWS(Expr::infer(ut.sorts,ut.ops,"cmp",{g,f}));

}
TEST_CASE("expr parser and printer") {
    Theory t=natarray();
    Ve xs = t.parse_exprs("data/arrayterms.dat");
    for (auto x: xs){
        CHECK(t.parse_expr(t.print(x))==x);}

    Theory h = cat();
    Expr fg = h.parse_expr("(f:(A:Ob⇒B:Ob) ⋅g:(B:Ob ⇒ C:Ob))");

    CHECK(fg==h.parse_expr(h.print(fg)));
}



TEST_CASE("parse theory") {
    Theory t1 = Theory::parseTheory("data/cat.dat");
    Theory t2 = cat();
    CHECK(t1.print()==t2.print());
    CHECK(t1.sorts==t2.sorts);
    CHECK(t1.ops==t2.ops);
    CHECK(t1.rules==t2.rules);
    CHECK(t1==t2);

}

TEST_CASE("mk_freevar") {
    Expr s=Srt("X");
    Expr x=Var("x",s),y=Var("y",s),z=Var("z",s);
    Expr xy=App("+",{x,y}),xyz=App("f",{z,y,x});
    CHECK(xy.freevar(xyz).empty());
    std::map<std::string,int>m{{"z",1}};
    CHECK(xyz.freevar(xy) == m);
}
