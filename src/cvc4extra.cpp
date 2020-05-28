#include <fstream>
#include "cvc4extra.hpp"


CVC4::api::Term mkConst(const CVC4::api::Solver & slv,
                  const std::string & name,
                  const CVC4::api::Term & t) {
    CVC4::api::Term v = slv.mkConst(t.getSort(), name);
    slv.assertFormula(slv.mkTerm(CVC4::api::EQUAL, v,t));
    return v;
}

CVC4::api::Term ITE(const CVC4::api::Solver & slv,
                    const Vt & ifs,
                    const Vt & thens,
                    const CVC4::api::Term & otherwise){
    if (ifs.size()!=thens.size())
        throw std::runtime_error("Called ITE with unequal ifs/thens");

    CVC4::api::Term ret=otherwise;
    for (auto i = ifs.size(); i--;){
        ret = slv.mkTerm(CVC4::api::ITE,ifs.at(i), thens.at(i), ret);
    }
    return ret;
}

void writeModel(CVC4::api::Solver & slv, std::string pth) {
    CVC4::api::Result res = slv.checkSat();
    if (res.isSat()){ // Write model to file
        std::ofstream outfile;
        outfile.open("build/"+pth);
        slv.printModel(outfile);
        outfile.close();
    }
}