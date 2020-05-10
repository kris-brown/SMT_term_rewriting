
#include <src/theory.hpp>

Theory cat() {
    Expr Ob=Sort("Ob");
    Expr A=Var("A",Ob), B=Var("B",Ob), C=Var("C",Ob), D=Var("D",Ob);
    Expr HomAA=Sort("Hom",{A,A}), HomAB=Sort("Hom",{A,B}), HomBC=Sort("Hom",{B,C}), HomCD=Sort("Hom",{C,D}), HomAC=Sort("Hom",{A,C});
    Expr idA=App("id",{A}),idB=App("id",{B});
    Expr f=Var("f",HomAB),g=Var("g",HomBC),h=Var("h",HomCD);
    Expr idf=App("cmp",{idA,f}),fid=App("cmp",{f,idB}),fg=App("cmp",{f,g}),gh=App("cmp",{g,h});
    Expr f_gh=App("cmp",{f,gh}),fg_h=App("cmp",{fg,h});

    SortDecl dOb{"Ob","Ob",{},"Objects in a category"};
    SortDecl dHom{"Hom","({}⇒{})",{A,B},"Hom-set of morphisms"};

    OpDecl idOp{"id","id({})",HomAA,{A},"Identity morphism"};
    OpDecl cmpOp{"cmp","({} ⋅ {})",HomAC,{f,g},"Identity morphism"};

    Rule idl{"idl","Left identity",f,idf};
    Rule idr{"idr","Right identity",f,fid};
    Rule asc{"asc","Associativity",f_gh,fg_h};

    std::map<std::string,SortDecl> sorts{{"Ob",dOb}, {"Hom",dHom}};
    std::map<std::string,OpDecl> ops{{"id",idOp},{"cmp",cmpOp}};
    std::vector<Rule> rules{idl,idr,asc};
    return {"cat", sorts,ops,rules};

}
