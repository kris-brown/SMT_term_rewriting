#include "../external/catch.hpp"
#include "../src/astextra.hpp"
#include "../src/theories/theories.hpp"



TEST_CASE("patfun") {
    CVC::Solver slv;
    slv.setOption("produce-models", "true");
    Theory t=upgradeT(cat());

    CVC::Sort astSort;
    std::tie (astSort,std::ignore,std::ignore) = create_datatypes(slv,t,2);

    // id.f pattern matches with id.f
    Expr expr=t.rules.at(0).t2;
    CVC::Term x=construct(slv,astSort,t,expr);
    CVC::Term p=pat_fun(slv,t,x,1,"r");
    CHECK(slv.checkEntailed(p).isEntailed());
    // f doesn't pattern match with id.f
    Expr expr2=t.rules.at(0).t1;
    CVC::Term x2=construct(slv,astSort,t,expr2);
    CVC::Term p2=slv.mkTerm(CVC::NOT, pat_fun(slv,t,x2,1,"r"));
    CHECK(slv.checkEntailed(p2).isEntailed());
    // id.(f.g.h) pattern matches with id.f
    Expr fgh=uninfer(t.rules.at(2).t1);
    Expr ida=uninfer(expr.args.at(1));
    Expr expr3=upgrade(t, App("cmp",{ida,fgh}));
    CVC::Term x3=mkConst(slv,"x3",construct(slv,astSort,t,expr3));
    CVC::Term p3=pat_fun(slv,t,x3,1,"r");
    CHECK(slv.checkEntailed(p3).isEntailed());
    writeModel(slv,"test/patfun.dat");
}

TEST_CASE("rterm_fun") {
    Theory t=upgradeT(cat());
    CVC::Solver slv;
    slv.setOption("produce-models", "true");
    CVC::Sort astSort;
    std::tie (astSort,std::ignore,std::ignore) = create_datatypes(slv,t,2);

    Expr f=t.rules.at(0).t1, idf=t.rules.at(0).t2;
    CVC::Term f1 = construct(slv,astSort,t,f);
    CVC::Term idf1 = mkConst(slv,"idf1",construct(slv,astSort,t,idf));
    CVC::Term idf2 = mkConst(slv,"idf2",rterm_fun(slv,t,f1,0,1,"f"));
    CVC::Term x1 = slv.mkTerm(CVC::EQUAL,idf1,idf2);
    CHECK(slv.checkEntailed(x1).isEntailed());

    writeModel(slv,"test/rtermfun.dat");

}

TEST_CASE("getAt") {
    CVC::Solver slv;
    slv.setOption("produce-models", "true");
    Theory t=upgradeT(cat());

    Expr f_gh=t.rules.at(2).t1;
    Expr f = f_gh.args.at(1);
    Expr g = f_gh.args.at(2).args.at(1);

    Vvvi paths = all_paths(2,2);
    CVC::Sort astSort, pathSort;
    std::tie (astSort,pathSort,std::ignore) = create_datatypes(slv,t,2);

    CVC::Term f_gh1 = construct(slv,astSort,t,f_gh);
    CVC::Term f1 = construct(slv,astSort,t,f);
    CVC::Term g1 = construct(slv,astSort,t,g);
    CVC::Term pEmpty = unit(slv,pathSort,"Empty");
    CVC::Term p1 = unit(slv,pathSort,"P1");
    CVC::Term p21 = unit(slv,pathSort,"P21");
    CVC::Term p22 = unit(slv,pathSort,"P22");

    CVC::Term f_gh2 = getAt(slv,f_gh1,pEmpty,paths);
    CVC::Term f2 = getAt(slv,f_gh1,p1,paths);
    CVC::Term g2 = getAt(slv,f_gh1,p21,paths);
    CVC::Term h2 = getAt(slv,f_gh1,p22,paths);

    CVC::Term x1=slv.mkTerm(CVC::EQUAL,f_gh1,f_gh2);
    CVC::Term x2=slv.mkTerm(CVC::EQUAL,f1,f2);
    CVC::Term x3=slv.mkTerm(CVC::EQUAL,g1,g2);
    CVC::Term x4=slv.mkTerm(CVC::NOT,
                            slv.mkTerm(CVC::EQUAL,g1,h2));
    CHECK(slv.checkEntailed(x1).isEntailed());
    CHECK(slv.checkEntailed(x2).isEntailed());
    CHECK(slv.checkEntailed(x3).isEntailed());
    CHECK(slv.checkEntailed(x4).isEntailed());


    writeModel(slv,"test/getat.dat");

}


TEST_CASE("replaceAt") {
    CVC::Solver slv;
    slv.setOption("produce-models", "true");
    Theory t=upgradeT(monoid());
    Vvvi paths = all_paths(2,2);
    CVC::Sort astSort, pathSort;
    std::tie (astSort,pathSort,std::ignore) = create_datatypes(slv,t,2);


    Expr x_yz=t.rules.at(2).t1, x=x_yz.args.at(1);
    Expr u_xyz = uninfer(x_yz);
    Expr ux = uninfer(x), uy = u_xyz.args.at(1).args.at(0);
    Expr x_yx=upgrade(t,App("M",{ux,App("M",{uy,ux})}));
    CVC::Term xyz_=mkConst(slv,"xyz",construct(slv,astSort,t,x_yz));
    CVC::Term xyx_=mkConst(slv,"xyx",construct(slv,astSort,t,x_yx));
    CVC::Term x_=mkConst(slv,"x",construct(slv,astSort,t,x));
    CVC::Term p22 = unit(slv,pathSort,"P22");
    CVC::Term pEmpty = unit(slv,pathSort,"Empty");
    CVC::Term result = mkConst(slv,"result",replaceAt(slv,xyz_,x_,p22,paths));
    CVC::Term x2 = replaceAt(slv,xyz_,x_,pEmpty,paths);
    CVC::Term b1 = slv.mkTerm(CVC::EQUAL,xyx_,result);
    CVC::Term b2 = slv.mkTerm(CVC::EQUAL,x_,x2);
    CHECK(slv.checkEntailed(b1).isEntailed());
    CHECK(slv.checkEntailed(b2).isEntailed());
    writeModel(slv,"test/replaceat.dat");

}


TEST_CASE("rewriteTop") {

}
TEST_CASE("assert_rewrite") {

}