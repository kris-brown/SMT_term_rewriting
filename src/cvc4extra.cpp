#include <fstream>
#include "cvc4extra.hpp"
#include "smt-switch/cvc4_factory.h"


smt::Term mkConst(const smt::SmtSolver & slv,
                  const std::string & name,
                  const smt::Term & t) {
    smt::Term v = slv->make_symbol(name, t->get_sort());
    slv->assert_formula(slv->make_term(smt::Equal, v,t));
    return v;
}

smt::Term ITE(const smt::SmtSolver & slv,
                    const Vt & ifs,
                    const Vt & thens,
                    const smt::Term & otherwise){
    if (ifs.size()!=thens.size())
        throw std::runtime_error("Called ITE with unequal ifs/thens");

    smt::Term ret=otherwise;
    for (auto i = ifs.size(); i--;){
        ret = slv->make_term(smt::Ite,ifs.at(i), thens.at(i), ret);
    }
    return ret;
}

void writeModel(smt::SmtSolver & slv, std::string pth) {
    smt::Result res = slv->check_sat();
    if (res.is_sat()){ // Write model to file
        //smt::CVC4Solver cslv = std::static_pointer_cast<smt::CVC4Solver>(slv);
        //FOR SOME REASON NOT IN NAMESPACE?
        //cslv.solver.writeModel("build/"+pth);
    }
}