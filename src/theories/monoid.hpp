#ifndef THEORY_MONOID
#define THEORY_MONOID

#include "../theory.hpp"

// https://en.wikipedia.org/wiki/Monoid

Theory monoid() {
    Expr Ob=Srt("Ob"),x=Var("x",Ob),y=Var("y",Ob),z=Var("z",Ob),e=App("e");
    SortDecl dOb{"Ob","Ob",{},"Some set"};
    OpDecl mOp{"M","({}⋅{})",Ob,{x,y}, "Multiplication"};
    OpDecl eOp{"e","e",Ob,{}, "Identity element"};
    Rule idl{"Left identity","",x,App("M",{e,x})};
    Rule idr{"Right identity","",x,App("M",{x,e})};
    Rule asc{"Associativity","",App("M",{x,App("M",{y,z})}), App("M",{App("M",{x,y}),z})};
    return {"monoid",{dOb},{mOp,eOp},{idl,idr,asc}};
}

#endif
