#include <vector>
#include <string>
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
/*
 * Accept user input and check (for a finite set of possible steps)
 *if a rewrite is possible modulo the axioms a theory.
 */

/**
 * Prepare theory from user input.
 * @param t either a theory given as a path or a named pre-existing theory
 * @returns upgraded theory to be used for term rewriting
 */
Theory input_theory(const std::string t)
{
    std::ifstream infile(t);
    if (infile.fail())
    {
        infile.close();
        return get_theory(t).upgrade();
    }
    else
    {
        return Theory::parseTheory(t).upgrade();
    }
}

std::string sep = "\n*******************************************\n";

int main(int argc, char **argv)
{
    // Values to be provided by user input
    std::string theoryname, term1, term2, depthstr, stepsstr;
    int depth, steps;

    // Get user input
    std::cout << "Give the name of Generalized Algebraic Theory (or path to file): ";
    getline(std::cin, theoryname);
    Theory t = input_theory(theoryname);

    std::cout << "Give an initial term in this theory: ";
    getline(std::cin, term1);
    Expr initial_term = t.upgrade(t.parse_expr(term1));

    std::cout << "Give a final term in this theory: ";
    getline(std::cin, term2);
    assert(term1 != term2);
    Expr final_term = t.upgrade(t.parse_expr(term2));

    std::cout << "Give max number rewrite steps: ";
    getline(std::cin, stepsstr);
    steps = std::stoi(stepsstr);

    std::cout << "Give a maximum depth for applying rewrites: ";
    getline(std::cin, depthstr);
    depth = std::stoi(depthstr);

    // Initialize a SMT-switch solver
    smt::SmtSolver slv = smt::CVC4SolverFactory::create(false);
    slv->set_opt("produce-models", "true");

    std::cout << "\n\nComputing...\n"
              << std::endl;

    // Declare datatypes
    smt::Sort astSort, pathSort, ruleSort;
    std::tie(astSort, pathSort, ruleSort) = create_datatypes(slv, t, depth);

    // Declare initial and final terms to the solver as constants
    smt::Term c1 = construct(slv, astSort, t, initial_term);
    smt::Term c2 = construct(slv, astSort, t, final_term);
    smt::Term x1 = mkConst(slv, "initial", c1);
    smt::Term x2 = mkConst(slv, "final", c2);

    // Create transition system
    pono::FunctionalTransitionSystem fts(slv);
    smt::Term state = fts.make_statevar("x", astSort);
    smt::Sort Int = slv->make_sort(smt::INT);
    smt::Term zero = slv->make_term(0, Int);
    smt::Term one = slv->make_term(1, Int);

    //Need counter to allow generating fresh free vars each iteration
    smt::Term cnt = fts.make_statevar("cnt", Int);

    // Initial state
    fts.constrain_init(slv->make_term(smt::Equal, state, c1));
    fts.constrain_init(slv->make_term(smt::Equal, cnt, zero));

    // Variable inputs for each transition
    smt::Term r = fts.make_inputvar("r", ruleSort);
    smt::Term p = fts.make_inputvar("p", pathSort);

    // Transition rule
    fts.assign_next(cnt, slv->make_term(smt::Plus, cnt, one));
    fts.assign_next(state, rewrite(slv, t, state, r, p, cnt, depth));

    // End goal to demonstrate
    smt::Term prop = slv->make_term(
        smt::Not, slv->make_term(smt::Equal, state, c2));

    // Final setup
    pono::Property property(fts, prop);
    pono::Bmc bmc(property, slv);
    std::vector<smt::UnorderedTermMap> wit;

    // Do the model checking
    switch (bmc.check_until(steps))
    {
    case pono::FALSE:
        bmc.witness(wit);
        std::cout << "\n"
                  << wit.size() - 1 << "-step solution found" << std::endl;
        std::cout << "\n\nStarting from " << term1 << std::endl;
        for (int i = 0; i + 1 != wit.size(); i++)
        {
            std::string rval = wit.at(i).at(r)->to_string();
            Rule rule = t.rules.at(std::stoi(rval.substr(1, rval.size() - 2)) - 1);
            Expr parsed = parseCVC(t, wit.at(i + 1).at(state)->to_string());
            int forward = (int)(rval.back() == 'f');
            std::cout << sep << "Step " << i << ": apply " << rval << " ("
                      << (forward ? "forward" : "reverse")
                      << ")\n"
                      << t.print(rule, forward) << "\nat subpath "
                      << wit.at(i).at(p) << " to yield:\n\t"
                      << t.print(parsed.uninfer()) << std::endl;
        }
        break;

    case pono::TRUE:
        std::cout << "\nNo rewrite possible" << std::endl;
        break;

    case pono::ERROR:
        std::cout << "\nError" << std::endl;
        break;

    case pono::UNKNOWN:
        std::cout << "\nNo solution found" << std::endl;
        break;
    }

    writeModel(slv, "model.dat");
    return 0;
}
