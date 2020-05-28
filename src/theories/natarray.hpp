#ifndef THEORY_NATARRAY
#define THEORY_NATARRAY

#include "../theory.hpp"

// Arrays indexed by naturals which store objects of some type. We can read/write to the arrays.

Theory natarray() {
    // Useful shorthand Exprs
    Expr Ob=Srt("Ob"), Nat=Srt("Nat"), Bool=Srt("Bool"), Arr=Srt("Arr");
    Expr o=Var("o", Ob), p=Var("p", Ob), A=Var("A", Arr), i=Var("i", Nat), j=Var("j", Nat), b=Var("b", Bool);
    Expr Si=App("S",{i}),Z=App("Z"),ZeZ=App("E",{Z,Z});
    Expr True=App("T"),False=App("F");

    // Declarations
    SortDecl dOb{"Ob","Ob"},dNat{"Nat","N"},dBool{"Bool","Bool"},dArr{"Arr","Arr"};
    OpDecl zeroOp{"Z","0",Nat,{},"Zero"};
    OpDecl trueOp{"T","⊤",Bool,{},"Truth"};
    OpDecl falseOp{"F", "⊥",Bool,{},"Falsum"};
    OpDecl sucOp{"S","S({})",Nat,{i},"Successor"};
    OpDecl eqOp{"E","({}≡{})",Bool,{i,j},"Nat equality"};
    OpDecl iteOp{"ite","ite({},{},{})",Ob,{b,o,p},"if-then-else"};
    OpDecl readOp{"read","read({},{})",Ob,{A,i},"read at position"};
    OpDecl writeOp{"write","write({},{},{})",Arr,{A,i,o},"write to position"};

    Rule row{"Read over write","If indices not equal, look at previous write.",
             App("read",{App("write",{A,i,o}), j}),
             App("ite",{App("E",{i,j}),o,App("read",{A,j})})};
    Rule eq1{"Eq1","Recursively remove successor ops",App("E",{i,j}),App("E",{Si,App("S",{j})})};
    Rule eq2{"Eq2","Base case:Zero equals zero", ZeZ, True};
    Rule eq3{"Eq3","Base case: Zero and something", App("E",{Si,Z}),False};
    Rule eq4{"Eq4","Base case: Something and zero", App("E",{Z,Si}),False};
    Rule if1{"If1","If then else, true scenario", App("ite",{True,o,p}),o};
    Rule if2{"If2","If then else, false scenario", App("ite",{False,o,p}),p};

    // Summary
    std::vector<SortDecl> sorts{dOb, dNat, dBool, dArr};
    std::vector<OpDecl> ops{
        zeroOp,trueOp,falseOp,sucOp,eqOp,iteOp,readOp,writeOp};
    std::vector<Rule> rules{row,eq1,eq2,eq3,eq4,if1,if2};

    return {"natarray", sorts,ops,rules};
}

#endif
