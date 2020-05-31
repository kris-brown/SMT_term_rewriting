#include<vector>
#include<string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "core/fts.h"
#include "engines/bmc.h"
#include "smt-switch/smt.h"
#include "smt-switch/cvc4_factory.h"
#include "cvc4extra.hpp"
#include "astextra.hpp"
#include "theory.hpp"
#include "theories/theories.hpp"


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
    Expr t1=upgrade(t, parse_expr(t, term1));
    std::cout << "Give a final term in this theory: ";
    getline(std::cin, term2);
    Expr t2=upgrade(t,parse_expr(t,term2));
    std::cout << "Give how many rewrite steps to expect: ";
    getline(std::cin, stepsstr);
    steps=std::stoi(stepsstr);
    std::cout << "Give a maximum depth for applying rewrites: ";
    getline(std::cin, depthstr);
    depth=std::stoi(depthstr);

    std::string code=symcodestr(t);
    smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);
    slv->set_opt("produce-models", "true");
    //mkConst(slv,"dic",slv.mkString(code, true));

    std::cout << "\n\nComputing...\n" << std::endl;

    // Declare datatypes
    smt::Sort astSort,pathSort,ruleSort;
    std::tie (astSort,pathSort,ruleSort) = create_datatypes(slv,t,depth);
    //writeModel(slv,"model.dat");


    smt::Term c1=construct(slv,astSort,t,t1), c2=construct(slv,astSort,t,t2);
    smt::Term x1=mkConst(slv,"initial",c1), x2=mkConst(slv,"final",c2);


    cosa::FunctionalTransitionSystem fts(slv);
    smt::Term state = fts.make_state("x", astSort);
    smt::Sort Int = slv->make_sort(smt::INT);
    smt::Term cnt = fts.make_state("cnt", Int);
    fts.constrain_init(slv->make_term(smt::Equal, state, x1));
    fts.constrain_init(slv->make_term(smt::Equal, cnt, slv->make_term(0, Int)));
    smt::Term r = fts.make_input("r", ruleSort);
    smt::Term p = fts.make_input("p", pathSort);
    smt::Term prop = slv->make_term(
        smt::Not, slv->make_term(smt::Equal,state, x2));

    cosa::Property property(fts, prop);
    cosa::Bmc bmc(property, slv);
    std::string div="\n**************************************\n";

    std::vector<smt::UnorderedTermMap> wit;

    switch (bmc.check_until(5)) {
       case cosa::FALSE :
        std::cout << "\nSolution found" << std::endl;
        bmc.witness(wit);
        for (int i=0;i!=wit.size();i++) {
            smt::UnorderedTermMap vars=wit.at(i);
            std::string rval = vars.at(r)->to_string();
            Rule rule=t.rules.at(std::stoi(rval.substr(1,rval.size()-2))-1);
            Expr parsed=parseCVC(t,vars.at(state)->to_string());
            std::cout << div << "Step " << i << ": apply " << rval << " ("
                    << ((rval.back()=='f') ? "forward" : "reverse")
                    << ")\n" << print(t,rule) << "\nat subpath "
                    << vars.at(p) << " to yield:\n\t"
                    << print(t,uninfer(parsed)) << std::endl;

        }

        break;
       case cosa::TRUE :
        std::cout << "\nNo rewrite possible" << std::endl;
        break;
        case cosa::UNKNOWN :
            std::cout << "\nNo solution found" << std::endl;
        break;
   }


    writeModel(slv,"model.dat");
    return 0;
}


