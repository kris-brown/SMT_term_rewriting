#include "../external/catch.hpp"
#include <fstream>
#include "../src/astextra.hpp"
#include "../src/cvc4extra.hpp"

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

TEST_CASE("mk_freevar") {
    Expr s=Sort("X");
    Expr x=Var("x",s),y=Var("y",s),z=Var("z",s);
    Expr xy=App("+",{x,y}),xyz=App("f",{z,y,x});
    CHECK(mk_freevar(xy,xyz).empty());
    std::map<std::string,int>m{{"z",1}};
    CHECK(mk_freevar(xyz,xy) == m);
}

TEST_CASE("replace1"){
    CVC::Solver slv;
    slv.setOption("produce-models", "true");

    Theory t; // EMPTY THEORY
    Vt asts, repls;
    CVC::Term node,astCon,noneterm;
    std::tie(std::ignore,std::ignore,std::ignore,std::ignore, std::ignore, std::ignore, std::ignore,
        astCon, node, std::ignore, std::ignore, noneterm, std::ignore, std::ignore,
        std::ignore,std::ignore,repls,asts) = setup(slv,t,false,false,false);
    CVC::Term l7 = slv.mkTerm(CVC::APPLY_UF,asts.at(1),slv.mkReal(7), noneterm);
    CVC::Term node_l7 = slv.mkTerm(CVC::APPLY_SELECTOR, node, l7);
    CVC::Term l8 = slv.mkTerm(CVC::APPLY_UF,asts.at(1),slv.mkReal(8), noneterm);
    CVC::Term l17 = mkConst(slv, "l17", slv.mkTerm(CVC::APPLY_UF, asts.at(1), slv.mkReal(1), l7));
    CVC::Term l18 = mkConst(slv, "l18", slv.mkTerm(CVC::APPLY_UF, asts.at(1), slv.mkReal(1), l8));
    CVC::Term subl18 = mkConst(slv, "l178", slv.mkTerm(CVC::APPLY_UF, repls.at(0), l17, l8)) ;


    // Access of node
    CVC::Term assert1 = slv.mkTerm(CVC::EQUAL, node_l7, slv.mkReal(7));

    // Simple substitution
    CVC::Term assert2 = slv.mkTerm(CVC::EQUAL, l18, subl18);

    CHECK(slv.checkEntailed(assert1).isEntailed());
    CHECK(slv.checkEntailed(assert2).isEntailed());

    writeModel(slv, "test/replacetest.dat"); // Log output model
 }
