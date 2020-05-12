#include "../theory.hpp"

// Boolean algebra

Theory boolalg() {
    Expr Bool=Sort("Bool");
    Expr b = Var("b",Bool),x = Var("x",Bool),y = Var("y",Bool),z = Var("z",Bool);

   SortDecl dBool{"Bool","Bool",{},"Underlying set"};
   OpDecl dTop{"T","⊤",Bool};
   OpDecl dBot{"F","⊥",Bool};
   OpDecl dNeg{"N","(¬{})",Bool,{b}};
   OpDecl dAnd{"A","({}∧{})",Bool,{b,b}};
   OpDecl dOr{"O","({}∨{})",Bool,{b,b}};

    auto mkId = [=](std::string op1,std::string op2) {
        return Rule{op1+" identity", "", b, App(op1,{b,App(op2)})};};

    auto mkSym = [=](std::string op) {
        return Rule{op+" symmetry", "", App(op,{x,y}), App(op,{y,x})};};

    auto mkAsc  = [=](std::string op) {
        return Rule{op+" associativity", "", App(op,{x,App(op,{y,z})}), App(op,{App(op,{x,y}),z})};};
    auto mkDist = [=](std::string op1,std::string op2) {
        return Rule{op1+" distributivity", "", App(op1,{x,App(op2,{y,z})}), App(op2,{App(op1,{x,y}),App(op1,{x,z})})};};
    auto mkDef = [=](std::string op1,std::string op2) {
        return Rule{op1+" definition", "", App(op1), App(op2,{b,App("N",{b})})};};


    Rule idA = mkId("A","T");
    Rule idO = mkId("O","F");
    Rule symA = mkSym("A");
    Rule symO = mkSym("O");
    Rule ascA = mkAsc("A");
    Rule ascO = mkAsc("O");
    Rule distA = mkDist("A","O");
    Rule distO = mkDist("O","A");
    Rule topDef = mkDef("T","O");
    Rule botDef = mkDef("F","A");

    std::vector<SortDecl> sorts{dBool};
    std::vector<OpDecl> ops{dTop,dBot,dNeg,dAnd,dOr};
    std::vector<Rule> rules{idA,idO,symA,symO,ascA,ascO,distA,distO};

    return {"boolalg", sorts,ops,rules};
}