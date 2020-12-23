#include <fstream>
#include <iostream>
#include "cvc4extra.hpp"
#include "smt-switch/cvc4_factory.h"
#include "smt-switch/cvc4_solver.h"

smt::Term mkConst(const smt::SmtSolver &slv,
                  const std::string &name,
                  const smt::Term &t)
{
    // Define a constant of the correct sort
    smt::Term v = slv->make_symbol(name, t->get_sort());
    // Assert the term is equal to it
    slv->assert_formula(slv->make_term(smt::Equal, v, t));
    return v;
}

smt::Term ITE(const smt::SmtSolver &slv,
              const Vt &ifs,
              const Vt &thens,
              const smt::Term &otherwise)
{
    if (ifs.size() != thens.size())
        throw std::runtime_error("Called ITE with unequal ifs/thens");

    smt::Term ret = otherwise;
    for (auto i = ifs.size(); i--;)
    {
        ret = slv->make_term(smt::Ite, ifs.at(i), thens.at(i), ret);
    }
    return ret;
}

void writeModel(smt::SmtSolver &slv, std::string pth)
{
    smt::Result res = slv->check_sat();
    if (res.is_sat())
    {
        CVC4::api::Solver &cvc4slv = std::static_pointer_cast<smt::CVC4Solver>(slv)->get_cvc4_solver();
        std::ofstream outfile;
        outfile.open("build/" + pth);
        cvc4slv.printModel(outfile);
        outfile.close();
    }
}

Res printResult(std::vector<smt::UnorderedTermMap> maps)
{
    Res all_res;
    for (auto map : maps)
    {
        ResStep res;
        for (auto pair : map)
            res.push_back({pair.first->to_string(), pair.second->to_string()});
        std::sort(res.begin(), res.end());
        all_res.push_back(res);
    }
    return all_res;
}
