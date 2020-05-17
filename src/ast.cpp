#include<vector>
#include<string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <cvc4/api/cvc4cpp.h>
#include <src/cvc4extra.hpp>
#include <src/astextra.hpp>
#include <src/theory.hpp>
#include <src/theories/theories.hpp>


Theory input_theory(const std::string t) {
    std::ifstream infile(t);
    if (infile.fail()){
        infile.close();
        return upgradeT(get_theory(t));
    } else {
        return upgradeT(parseTheory(t));
    }
}

int main(int argc, char** argv)
{
    std::string theoryname,term1,term2,depthstr,stepsstr;
    int depth, steps;
    std::cout << "Give the name of Generalized Algebraic Theory (or path to file): ";
    getline(std::cin, theoryname);
    Theory t = input_theory(theoryname);
    std::cout << "Give an initial term in this theory: ";
    getline(std::cin, term1);
    Expr t1=parse_expr(t, term1);
    std::cout << "Give a final term in this theory: ";
    getline(std::cin, term2);
    Expr t2=parse_expr(t,term2);
    std::cout << "Give how many rewrite steps to expect: ";
    getline(std::cin, stepsstr);
    steps=std::stoi(stepsstr);
    std::cout << "Give a maximum depth for applying rewrites: ";
    getline(std::cin, depthstr);
    depth=std::stoi(depthstr);

    CVC::Solver slv;
    slv.setOption("produce-models", "true");
    // Declare datatypes
    CVC::Sort astSort,pathSort,ruleSort;
    std::tie (astSort,pathSort,ruleSort) = create_datatypes(slv,t,depth);


    CVC::Term c1=construct(slv,astSort,t,t1),c2=construct(slv,astSort,t,t2);
    CVC::Term x1=mkConst(slv,"initial",c1), x2=mkConst(slv,"final",c2);

    CVC::Term r=assert_rewrite(slv, astSort, pathSort,ruleSort, t, x1, x2, steps);
    slv.assertFormula(r);

    if (!slv.checkSat().isSat()) {
        std::cout << "\nNo solution found" << std::endl;
        slv.resetAssertions();
        CVC::Term x1=mkConst(slv,"init",c1), x2=mkConst(slv,"fin",c2);
        slv.assertFormula(slv.mkTerm(CVC::NOT,r));
    } else {
        std::cout << "\nSolution found" << std::endl;
    }

    writeModel(slv,"model.dat");
    return 0;
}


