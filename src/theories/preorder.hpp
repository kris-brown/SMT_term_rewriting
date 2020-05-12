#include "../theory.hpp"

// https://en.wikipedia.org/wiki/Preorder

Theory preorder() {
    Expr Ob=Sort("Ob"),A=Var("A",Ob),B=Var("B",Ob),C=Var("C",Ob);
    Expr aa=Sort("L",{A,A}),ab=Sort("L",{A,B}),bc=Sort("L",{B,C}),ac=Sort("L",{A,C});
    Expr p=Var("p",ab),q=Var("q",bc),r=Var("r",ab);
    SortDecl dOb{"Ob","Ob",{},"Some set"};
    SortDecl dLeq{"L","({}≤{})",{A,B},"A relation from A -> B"};
    OpDecl refl{"R","{}ᵣ",aa,{A},"Reflexivity"};
    OpDecl trans{"trans","({}⪯{})",ac,{p,q},"Transitivity"};
    Rule singl{"Singleton","The sort A->B is a singleton set.",p,r};
    return {"preorder",std::vector<SortDecl>{dOb,dLeq},{refl,trans},{singl}};
}