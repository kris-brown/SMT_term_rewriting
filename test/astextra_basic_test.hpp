#include "../external/catch.hpp"
#include "../src/astextra_basic.hpp"
#include "../src/astextra.hpp"
#include "../src/cvc4extra.hpp"
#include "../src/theories/theories.hpp"


TEST_CASE("cart_product") {
    Vvi input{{1,2},{1,2,3},{1},{1},{1,2}};
    Vi tmp;
    Vvi output;
    cart_product(output, tmp, input.begin(), input.end());
    CHECK(output.size() == 12);
}


TEST_CASE("paths_n") {
    // All paths of depth 3 with values 0/1/2
    CHECK(paths_n(3,2).size() == 27);
}

TEST_CASE("all_paths") {
    // All paths of depth 2 with values 0/1/2
    CHECK(all_paths(3,2).at(1).size() == 9);
}


TEST_CASE("replace1"){
    smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);

    slv->set_opt("produce-models", "true");

    Theory t=natarray();
    smt::Sort astSort, Int=slv->make_sort(smt::INT);
    std::tie (astSort,std::ignore,std::ignore) = create_datatypes(slv,t,2);

    smt::Datatype dt=astSort->get_datatype();
    CHECK(arity(astSort) == 4);
    smt::Term l7 = ast(slv,astSort, slv->make_term(7,Int));
    smt::Term l8 = ast(slv, astSort,  slv->make_term(8,Int));
    smt::Term node_l7 = node(slv, l7);
    smt::Term l17 = mkConst(slv, "l17", ast(slv, astSort,  slv->make_term(1,Int), {l7}));
    smt::Term l18 = mkConst(slv, "l18", ast(slv, astSort,  slv->make_term(1, Int), {l8}));
    smt::Term subl18 = mkConst(slv, "l178", replace(slv, 0, l17, l8)) ;

    // Access of node
    smt::Term assert1 = slv->make_term(smt::Equal, node_l7, slv->make_term(7,Int));
    // // Simple substitution
    smt::Term assert2 = slv->make_term(smt::Equal, l18, subl18);

    slv->assert_formula(assert1);
    slv->assert_formula(assert2);
    CHECK(slv->check_sat().is_sat());

    writeModel(slv, "test/replacet.dat"); // Log output model
 }

TEST_CASE("construct"){
     smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);

    slv->set_opt("produce-models", "true");

    Theory t=monoid();
    std::map<std::string,int> sc=symcode(t);

    smt::Sort astSort;
    std::tie (astSort,std::ignore,std::ignore) = create_datatypes(slv,t,2);
    smt::Sort Int = slv->make_sort(smt::INT);
    smt::Term ob1 = mkConst(slv,"Ob", ast(slv,astSort,slv->make_term(sc["Ob"],Int)));
    smt::Term x1 = mkConst(slv,"x",ast(slv, astSort,  slv->make_term(sc["x"],Int), {ob1}));
    smt::Term y1 = mkConst(slv,"y",ast(slv, astSort,  slv->make_term(sc["y"],Int), {ob1}));
    smt::Term q1 = mkConst(slv, "q",ast(slv,astSort,slv->make_term(strhash("q"),Int),{ob1}));
    Expr Ob=Srt("Ob"), x={"x",VarNode,{Ob}},y={"y",VarNode,{Ob}}, q={"q",VarNode,{Ob}};
    smt::Term x2 = mkConst(slv,"x2",construct(slv,astSort,t,x));
    smt::Term q2 = mkConst(slv,"q2",construct(slv,astSort,t,q));

    // Test constructions with empty context
    slv->assert_formula(slv->make_term(smt::Equal, x1, x2));
    slv->assert_formula(slv->make_term(smt::Equal, q1, q2));
    CHECK(slv->check_sat().is_sat());

    Expr xy=App("M",{x,y}), yx=App("M",{y,x});
    Expr xyq=App("M",{x,App("M",{y,q})}), qyx=App("M",{q,yx}), xyx=App("M",{x,yx});

    smt::Term xy1 = mkConst(slv,"xy", construct(slv,astSort,t,xy));

    smt::Term zzz = mkConst(slv,"zzz", construct(slv,astSort,t,xy));

    smt::Term yx1 = mkConst(slv,"yx",construct(slv,astSort,t,yx,xy,xy1));

    smt::Term xyx1 = mkConst(slv,"xyx",construct(slv,astSort,t,xyx,xy,xy1));
    smt::Term xyx2 = mkConst(slv,"xyx2",construct(slv,astSort,t,xyx));

    // Test constructions with context
    slv->assert_formula(slv->make_term(smt::Equal, xyx1, xyx2));
    CHECK(slv->check_sat().is_sat());

    writeModel(slv, "test/construct.dat"); // Log output model


}


TEST_CASE("replPfun") {
     smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);

    slv->set_opt("produce-models", "true");
    Theory t=upgradeT(monoid());
    Vvvi paths = all_paths(2,2);
    smt::Sort astSort, pathSort;
    std::tie (astSort,pathSort,std::ignore) = create_datatypes(slv,t,2);

    Expr Ob=Srt("Ob"),x=Var("x",Ob),y=Var("y",Ob);
    Expr xy=App("M",{x,y}), xx=App("M",{x,x});
    smt::Term xy_=mkConst(slv,"xy",construct(slv,astSort,t,upgrade(t,xy)));
    smt::Term xx_=mkConst(slv,"xx",construct(slv,astSort,t,upgrade(t,xx)));
    smt::Term x_=mkConst(slv,"x",construct(slv,astSort,t,upgrade(t,x)));
    smt::Term p2 = unit(slv,pathSort,"P2");
    smt::Term res = replP_fun(slv,xy_,x_,{2});
    smt::Term result=mkConst(slv,"result",res);
    smt::Term b1 = slv->make_term(smt::Equal,xx_,result);
    slv->assert_formula(b1);
    CHECK(slv->check_sat().is_sat());
    writeModel(slv,"test/replP.dat");

}

TEST_CASE("subterm") {

}

TEST_CASE("test") {

}

TEST_CASE("parseCVC") {
     smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);

    slv->set_opt("produce-models", "true");
    Theory t=upgradeT(cat());

    smt::Sort astSort;
    std::tie (astSort,std::ignore,std::ignore) = create_datatypes(slv,t,2);
    Expr expr=t.rules.at(0).t2;
    smt::Term x=mkConst(slv, "x", construct(slv,astSort,t,expr));
    slv->check_sat();
    std::string to_parse=slv->get_value(x)->to_string();
    Expr expr_= parseCVC(t,to_parse); // REMOVED SIMPLIFY...will cause a bug?
    CHECK(expr==expr_);

    //Now something with variables that are not in the theory
    Expr o=Srt("Ob"),q=Var("Q",{o}),z=Var("Z",{o});
    Expr h=Srt("Hom",{q,z}),m=Var("m",{h});
    smt::Term m_=mkConst(slv,"m",construct(slv,astSort,t,m));
    slv->check_sat();
    Expr x2_= parseCVC(t,slv->get_value(m_)->to_string());
    CHECK(m==x2_);
    writeModel(slv,"test/parsecvc.dat");

}


