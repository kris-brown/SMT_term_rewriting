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

std::string sep="\n*******************************************\n";


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
    assert(term1!=term2);
    Expr t2=upgrade(t,parse_expr(t,term2));
    std::cout << "Give max number rewrite steps: ";
    getline(std::cin, stepsstr);
    steps=std::stoi(stepsstr);
    std::cout << "Give a maximum depth for applying rewrites: ";
    getline(std::cin, depthstr);
    depth=std::stoi(depthstr);

    std::string code=symcodestr(t);
    smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);
    slv->set_opt("produce-models", "true");

    std::cout << "\n\nComputing...\n" << std::endl;

    // Declare datatypes
    smt::Sort astSort,pathSort,ruleSort;
    std::tie (astSort,pathSort,ruleSort) = create_datatypes(slv,t,depth);
    // Create initial and target terms
    smt::Term c1=construct(slv,astSort,t,t1), c2=construct(slv,astSort,t,t2);
    smt::Term x1=mkConst(slv,"initial",c1), x2=mkConst(slv,"final",c2);

    // Create transition system
    cosa::FunctionalTransitionSystem fts(slv);
    smt::Term state = fts.make_state("x", astSort);
    smt::Sort Int = slv->make_sort(smt::INT);
    smt::Term zero = slv->make_term(0, Int);
    smt::Term one = slv->make_term(0, Int);
    smt::Term cnt = fts.make_state("cnt", Int); // counter

    // Initial state
    fts.constrain_init(slv->make_term(smt::Equal, state, c1));
    fts.constrain_init(slv->make_term(smt::Equal, cnt, zero));

    // Variable inputs for each transition
    smt::Term r = fts.make_input("r", ruleSort);
    smt::Term p = fts.make_input("p", pathSort);

    // Transition rule
    fts.assign_next(cnt, slv->make_term(smt::Plus, cnt, one)); //increment
    fts.assign_next(state, rewrite(slv,t,state,r,p,cnt,depth));

    // End goal to demonstrate
    smt::Term prop = slv->make_term(
        smt::Not, slv->make_term(smt::Equal,state, c2));
    // Final setup
    cosa::Property property(fts, prop);
    cosa::Bmc bmc(property, slv);
    std::vector<smt::UnorderedTermMap> wit;

    // Do the model checking
    switch (bmc.check_until(steps)) {
       case cosa::FALSE:
        bmc.witness(wit);
        std::cout << "\n" << wit.size() -1 << "-step solution found" << std::endl;
        std::cout << "\n\nStarting from "<< term1 << std::endl;
        for (int i=0;i+1!=wit.size();i++) {
            std::string rval = wit.at(i).at(r)->to_string();
            Rule rule=t.rules.at(std::stoi(rval.substr(1,rval.size()-2))-1);
            Expr parsed=parseCVC(t,wit.at(i+1).at(state)->to_string());
            int forward=(int)(rval.back()=='f');
            std::cout << sep << "Step " << i << ": apply " << rval << " ("
                    << (forward ? "forward" : "reverse")
                    << ")\n" << print(t,rule,forward) << "\nat subpath "
                    << wit.at(i).at(p) << " to yield:\n\t"
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


