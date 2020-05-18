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
    CVC::Solver slv;
    slv.setOption("produce-models", "true");

    Theory t=natarray();
    CVC::Sort astSort;
    std::tie (astSort,std::ignore,std::ignore) = create_datatypes(slv,t,2);
    CHECK(arity(astSort) == 4);
    CVC::Term l7 = ast(slv,astSort, slv.mkReal(7));
    CVC::Term l8 = ast(slv, astSort,  slv.mkReal(8));
    CVC::Term node_l7 = node(slv, l7);
    CVC::Term l17 = mkConst(slv, "l17", ast(slv, astSort,  slv.mkReal(1), {l7}));
    CVC::Term l18 = mkConst(slv, "l18", ast(slv, astSort,  slv.mkReal(1), {l8}));
    CVC::Term subl18 = mkConst(slv, "l178", replace(slv, 0, l17, l8)) ;

    // Access of node
    CVC::Term assert1 = slv.mkTerm(CVC::EQUAL, node_l7, slv.mkReal(7));
    // // Simple substitution
    CVC::Term assert2 = slv.mkTerm(CVC::EQUAL, l18, subl18);

    CHECK(slv.checkEntailed(assert1).isEntailed());
    CHECK(slv.checkEntailed(assert2).isEntailed());

    writeModel(slv, "test/replacet.dat"); // Log output model
 }

TEST_CASE("construct"){
    CVC::Solver slv;
    slv.setOption("produce-models", "true");

    Theory t=monoid();
    std::map<std::string,int> sc=symcode(t);

    CVC::Sort astSort;
    std::tie (astSort,std::ignore,std::ignore) = create_datatypes(slv,t,2);

    CVC::Term ob1 = mkConst(slv,"Ob", ast(slv,astSort,slv.mkReal(sc["Ob"])));
    CVC::Term x1 = mkConst(slv,"x",ast(slv, astSort,  slv.mkReal(sc["x"]), {ob1}));
    CVC::Term y1 = mkConst(slv,"y",ast(slv, astSort,  slv.mkReal(sc["y"]), {ob1}));
    CVC::Term q1 = mkConst(slv, "q",ast(slv,astSort,slv.mkReal(strhash("q")),{ob1}));
    Expr Ob=Sort("Ob"), x={"x",VarNode,{Ob}},y={"y",VarNode,{Ob}}, q={"q",VarNode,{Ob}};
    CVC::Term x2 = mkConst(slv,"x2",construct(slv,astSort,t,x));
    CVC::Term q2 = mkConst(slv,"q2",construct(slv,astSort,t,q));

    // Test constructions with empty context
    CHECK(slv.checkEntailed(slv.mkTerm(CVC::EQUAL, x1, x2)).isEntailed());
    CHECK(slv.checkEntailed(slv.mkTerm(CVC::EQUAL, q1, q2)).isEntailed());

    Expr xy=App("M",{x,y}), yx=App("M",{y,x});
    Expr xyq=App("M",{x,App("M",{y,q})}), qyx=App("M",{q,yx}), xyx=App("M",{x,yx});

    CVC::Term xy1 = mkConst(slv,"xy", construct(slv,astSort,t,xy));

    CVC::Term zzz = mkConst(slv,"xy", construct(slv,astSort,t,xy));

    CVC::Term yx1 = mkConst(slv,"yx",construct(slv,astSort,t,yx,xy,xy1));

    CVC::Term xyx1 = mkConst(slv,"xyx",construct(slv,astSort,t,xyx,xy,xy1));
    CVC::Term xyx2 = mkConst(slv,"xyx2",construct(slv,astSort,t,xyx));

    // Test constructions with context
    CHECK(slv.checkEntailed(slv.mkTerm(CVC::EQUAL, xyx1, xyx2)).isEntailed());

    writeModel(slv, "test/construct.dat"); // Log output model


}


TEST_CASE("replPfun") {
    CVC::Solver slv;
    slv.setOption("produce-models", "true");
    Theory t=upgradeT(monoid());
    Vvvi paths = all_paths(2,2);
    CVC::Sort astSort, pathSort;
    std::tie (astSort,pathSort,std::ignore) = create_datatypes(slv,t,2);

    Expr Ob=Sort("Ob"),x=Var("x",Ob),y=Var("y",Ob);
    Expr xy=App("M",{x,y}), xx=App("M",{x,x});
    CVC::Term xy_=mkConst(slv,"xy",construct(slv,astSort,t,upgrade(t,xy)));
    CVC::Term xx_=mkConst(slv,"xx",construct(slv,astSort,t,upgrade(t,xx)));
    CVC::Term x_=mkConst(slv,"x",construct(slv,astSort,t,upgrade(t,x)));
    CVC::Term p2 = unit(slv,pathSort,"P2");
    CVC::Term res = replP_fun(slv,xy_,x_,{2});
    CVC::Term result=mkConst(slv,"result",res);
    CVC::Term b1 = slv.mkTerm(CVC::EQUAL,xx_,result);
    CHECK(slv.checkEntailed(b1).isEntailed());
    writeModel(slv,"test/replP.dat");

}

TEST_CASE("subterm") {

}

TEST_CASE("test") {

}

