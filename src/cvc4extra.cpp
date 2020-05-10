#include <src/cvc4extra.hpp>


CVC4::api::Term mkConst(const CVC4::api::Solver & slv,
                  const std::string & name,
                  const CVC4::api::Term & t) {
    CVC4::api::Term v = slv.mkConst(t.getSort(), name);
    slv.assertFormula(slv.mkTerm(CVC4::api::EQUAL, v,t));
    return v;
}

/*
Chain a series of IF-THEN pairs with an ELSE condition into one term.
*/
CVC4::api::Term ITE(const CVC4::api::Solver & slv,
                    const Vt & ifs,
                    const Vt & thens,
                    const CVC4::api::Term & otherwise){
    CVC4::api::Term ret=otherwise;
    for (auto i = ifs.size(); i--;){
        ret = slv.mkTerm(CVC4::api::ITE,ifs.at(i), thens.at(i), ret);
    }
    return ret;
}
