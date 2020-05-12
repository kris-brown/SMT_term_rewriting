#include<vector>
#include<string>
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

    const int depth = argc == 3 ? std::stoi(argv[2]) : 2; // DEFAULT TO 2
    const Theory t = upgradeT(get_theory(argv[1]));
    CVC::Solver slv;
    slv.setOption("produce-models", "true");
    setup(slv, t, depth);
    std::cout << "Expect sat. CVC4: " << slv.checkSat() << std::endl;
    writeModel(slv,"build/model.dat");
    return 0;
}


