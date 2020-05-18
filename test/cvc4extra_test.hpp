#include "../external/catch.hpp"
#include <cvc4/api/cvc4cpp.h>
#include "../src/cvc4extra.hpp"
namespace CVC = CVC4::api;

TEST_CASE("mkConst") {
    CVC::Solver slv;
    CHECK_NOTHROW(mkConst(slv,"x",slv.mkReal(9)));
}

TEST_CASE("ITE") {
    CVC::Solver slv;
    Vt thens{slv.mkReal(100),slv.mkReal(-100)};
    CVC :: Term Else= slv.mkReal(7);
    std::vector<int> args{-1,10,5}, res{-100,100,7};

    for (int i=0;i!=3;i++) {
        int not_i = (i+1) % 3;
        CVC::Term x=slv.mkReal(args.at(i));
        Vt ifs{slv.mkTerm(CVC::GT,x,slv.mkReal(9)),
               slv.mkTerm(CVC::LT,x,slv.mkReal(0))};
        CVC::Term t=ITE(slv,ifs,thens,Else);
        CVC::Result r1=slv.checkEntailed(slv.mkTerm(CVC::EQUAL,t,slv.mkReal(res.at(i))));
        CVC::Result r2=slv.checkEntailed(slv.mkTerm(CVC::EQUAL,t,slv.mkReal(res.at(not_i))));
        CHECK(r1.isEntailed());
        CHECK(r2.isNotEntailed());
    }

    // ifs and thens don't have same size
    CHECK_THROWS(ITE(slv,{Else,Else},{Else},Else));


}
