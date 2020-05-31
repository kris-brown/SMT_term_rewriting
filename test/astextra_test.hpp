#include "../external/catch.hpp"
#include "../src/astextra.hpp"
#include "../src/theories/theories.hpp"



TEST_CASE("patfun") {
     smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);

    slv->set_opt("produce-models", "true");
    Theory t=upgradeT(cat());

    smt::Sort astSort;
    std::tie (astSort,std::ignore,std::ignore) = create_datatypes(slv,t,2);

    // id.f pattern matches with id.f
    Expr expr=t.rules.at(0).t2;
    smt::Term x=construct(slv,astSort,t,expr);
    smt::Term p=pat_fun(slv,t,x,1,"r");
    slv->assert_formula(p);
    CHECK(slv->check_sat().is_sat());

    // f doesn't pattern match with id.f
    Expr expr2=t.rules.at(0).t1;
    smt::Term x2=construct(slv,astSort,t,expr2);
    smt::Term p2=slv->make_term(smt::Not, pat_fun(slv,t,x2,1,"r"));
    slv->assert_formula(p2);
    CHECK(slv->check_sat().is_sat());

    // id.(f.g.h) pattern matches with id.f
    Expr fgh=uninfer(t.rules.at(2).t1);
    Expr ida=uninfer(expr.args.at(1));
    Expr expr3=upgrade(t, App("cmp",{ida,fgh}));
    smt::Term x3=mkConst(slv,"x3",construct(slv,astSort,t,expr3));
    smt::Term p3=pat_fun(slv,t,x3,1,"r");
    slv->assert_formula(p3);
        CHECK(slv->check_sat().is_sat());

    writeModel(slv,"test/patfun.dat");
}

TEST_CASE("rterm_fun") {
    Theory t=upgradeT(cat());
     smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);

    slv->set_opt("produce-models", "true");
    smt::Sort astSort;
    std::tie (astSort,std::ignore,std::ignore) = create_datatypes(slv,t,2);

    Expr f=t.rules.at(0).t1, idf=t.rules.at(0).t2;
    smt::Term f1 = construct(slv,astSort,t,f);
    smt::Term idf1 = mkConst(slv,"idf1",construct(slv,astSort,t,idf));
    smt::Term idf2 = mkConst(slv,"idf2",rterm_fun(slv,t,f1,0,1,"f"));
    smt::Term x1 = slv->make_term(smt::Equal,idf1,idf2);
    slv->assert_formula(x1);
    CHECK(slv->check_sat().is_sat());

    writeModel(slv,"test/rtermfun.dat");

}

TEST_CASE("getAt") {
     smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);

    slv->set_opt("produce-models", "true");
    Theory t=upgradeT(cat());

    Expr f_gh=t.rules.at(2).t1;
    Expr f = f_gh.args.at(1);
    Expr g = f_gh.args.at(2).args.at(1);

    Vvvi paths = all_paths(2,2);
    smt::Sort astSort, pathSort;
    std::tie (astSort,pathSort,std::ignore) = create_datatypes(slv,t,2);

    smt::Term f_gh1 = construct(slv,astSort,t,f_gh);
    smt::Term f1 = construct(slv,astSort,t,f);
    smt::Term g1 = construct(slv,astSort,t,g);
    smt::Term pEmpty = unit(slv,pathSort,"Empty");
    smt::Term p1 = unit(slv,pathSort,"P1");
    smt::Term p21 = unit(slv,pathSort,"P21");
    smt::Term p22 = unit(slv,pathSort,"P22");

    smt::Term f_gh2 = getAt(slv,f_gh1,pEmpty,paths);
    smt::Term f2 = getAt(slv,f_gh1,p1,paths);
    smt::Term g2 = getAt(slv,f_gh1,p21,paths);
    smt::Term h2 = getAt(slv,f_gh1,p22,paths);

    smt::Term x1=slv->make_term(smt::Equal,f_gh1,f_gh2);
    smt::Term x2=slv->make_term(smt::Equal,f1,f2);
    smt::Term x3=slv->make_term(smt::Equal,g1,g2);
    smt::Term x4=slv->make_term(smt::Not,
                            slv->make_term(smt::Equal,g1,h2));
    slv->assert_formula(x1);
    slv->assert_formula(x2);
    slv->assert_formula(x3);
    slv->assert_formula(x4);
    CHECK(slv->check_sat().is_sat());


    writeModel(slv,"test/getat.dat");

}


TEST_CASE("replaceAt") {
     smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);

    slv->set_opt("produce-models", "true");
    Theory t=upgradeT(monoid());
    Vvvi paths = all_paths(2,2);
    smt::Sort astSort, pathSort;
    std::tie (astSort,pathSort,std::ignore) = create_datatypes(slv,t,2);


    Expr x_yz=t.rules.at(2).t1, x=x_yz.args.at(1);
    Expr u_xyz = uninfer(x_yz);
    Expr ux = uninfer(x), uy = u_xyz.args.at(1).args.at(0);
    Expr x_yx=upgrade(t,App("M",{ux,App("M",{uy,ux})}));
    smt::Term xyz_=mkConst(slv,"xyz",construct(slv,astSort,t,x_yz));
    smt::Term xyx_=mkConst(slv,"xyx",construct(slv,astSort,t,x_yx));
    smt::Term x_=mkConst(slv,"x",construct(slv,astSort,t,x));
    smt::Term p22 = unit(slv,pathSort,"P22");
    smt::Term pEmpty = unit(slv,pathSort,"Empty");
    smt::Term result = mkConst(slv,"result",replaceAt(slv,xyz_,x_,p22,paths));
    smt::Term x2 = replaceAt(slv,xyz_,x_,pEmpty,paths);
    smt::Term b1 = slv->make_term(smt::Equal,xyx_,result);
    smt::Term b2 = slv->make_term(smt::Equal,x_,x2);
    slv->assert_formula(b1);
    slv->assert_formula(b2);
    CHECK(slv->check_sat().is_sat());

    writeModel(slv,"test/replaceat.dat");

}


TEST_CASE("rewriteTop") {

}
TEST_CASE("assert_rewrite") {

}