#include<vector>
#include <fstream>
#include <iostream>

#include <cvc4/api/cvc4cpp.h>
#include <src/cvc4extra.hpp>
#include <src/astextra.hpp>
#include <src/theory.hpp>
#include <src/theories/theories.hpp>


int main(int argc, char** argv)
{
    if (argc < 2 || argc > 3)
        throw std::runtime_error("Must call with name of theory as argument (and, optionally, path depth)");

    // Setup
    //-------
    const int depth = argc == 3 ?  : 2; // HYPERPARAMETER
    const Theory t = upgradeT(get_theory(argv[1]));

    CVC::Solver slv;
    slv.setOption("produce-models", "true");

    CVC::Sort Int,astSort,pathSort,ruleSort;
    CVC::Datatype astDT,pathDT,ruleDT;
    CVC::Term astCon, node, isNone, isAST, noneterm, errterm, Empty;
    Vt as; Vi rules;
    Vt repls, asts;
    CVC::Term Node;
    CVC::Term getAt, replaceAt;

    std::tie(Int,astSort,pathSort,ruleSort, astDT, pathDT, ruleDT,
        astCon, node, isNone, isAST, noneterm, errterm, Empty,
        as,rules,repls,asts) = setup(slv, t, depth);

    CVC::Result res = slv.checkSat();
    std::cout << "Expect sat. CVC4: " << res << std::endl;

    if (res.isSat()){
        // Write model to file
        std::ofstream outfile;
        outfile.open("build/model.dat");
        slv.printModel(outfile);
        outfile.close();
    }
    return 0;
}

