#include <src/theory.hpp>

Theory intarray() {

    SortDecl dOb{"Ob","Ob"},dInt{"Int","Int"},dBool{"Bool","Bool"},dArr{"Arr","Arr"};
    Expr Ob=Sort("Ob"), Int=Sort("Int"), Bool=Sort("Bool"), Arr=Sort("Arr");
    Expr o=Var("o", Ob), p=Var("p", Ob), A=Var("A", Arr), i=Var("i", Int), j=Var("j", Int), b=Var("b", Bool);

    OpDecl zeroOp{"Z","0",Int,{},"Zero"};
    OpDecl trueOp{"T","⊤",Bool,{},"Truth"};
    OpDecl falseOp{"F", "⊥",Bool,{},"Falsum"};
    OpDecl sucOp{"S","S({})",Int,{i},"Successor"};
    OpDecl eqOp{"E","({}≡{})",Bool,{i,j},"Int equality"};
    OpDecl iteOp{"ite","ite({},{},{})",Ob,{b,o,p},"if then else"};
    OpDecl readOp{"read","read({},{})",Ob,{A,i},"read at position"};
    OpDecl writeOp{"write","write({},{},{})",Arr,{A,i,o},"write to position"};

    Expr Si=App("S",{i}),Z=App("Z"),ZeZ=App("E",{Z,Z});
    Expr True=App("T"),False=App("F");

    Rule row{"Read over write","",
             App("read",{App("write",{A,i,o}), j}),
             App("ite",{App("E",{i,j}),o,App("read",{A,j})})};
    Rule eq1{"Eq1","",ZeZ,True};
    Rule eq2{"Eq1","",App("E",{i,j}),App("E",{Si,App("S",{j})})};
    Rule eq3{"Eq3","",App("E",{Si,Z}),False};
    Rule eq4{"Eq4","",App("E",{Z,Si}),False};
    Rule if1{"If1","",App("ite",{True,o,p}),o};
    Rule if2{"If2","",App("ite",{False,o,p}),p};

    std::map<std::string,SortDecl> sorts{
        {"Ob",dOb},{"Int",dInt},{"Bool",dBool},{"Arr",dArr}};
    std::map<std::string,OpDecl> ops{
        {"Z",zeroOp},{"T",trueOp},{"F",falseOp},{"S",sucOp},{"E",eqOp},
        {"ite",iteOp},{"read",readOp},{"write",writeOp}};
    std::vector<Rule> rules{row,eq1,eq2,eq3,eq4,if1,if2};

    return {"intarray", sorts,ops,rules};
}

