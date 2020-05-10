#include<vector>
#include <fstream>
#include <iostream>

#include <cvc4/api/cvc4cpp.h>
#include <src/cvc4extra.hpp>
#include <src/astextra.hpp>
#include <src/theory.hpp>
#include <src/theories/theories.hpp>


int main(int argc, char** argv)
{
    // Setup
    //-------
    const int depth = 2; // HYPERPARAMETER
    const char fr[2]  = {'f','r'}; // Forward/reverse
    Theory t = upgradeT(get_theory(argv[1]));
    const int arity= max_arity(t);
    const Vvvi paths = all_paths(depth,arity);
    std::map<std::string,int> syms = symcode(t);

    CVC::Solver slv;
    slv.setOption("produce-models", "true");

    // Declare datatypes
    //----------------------
    CVC::Sort Int = slv.getIntegerSort();

    // AST
    CVC::DatatypeDecl astSpec = slv.mkDatatypeDecl("AST");
    CVC::DatatypeConstructorDecl ast("ast");
    ast.addSelector("node", Int);
    for (int i=0; i<=arity;i++)
        ast.addSelectorSelf("a"+std::to_string(i));

    astSpec.addConstructor(ast);
    astSpec.addConstructor(CVC::DatatypeConstructorDecl("None"));
    astSpec.addConstructor(CVC::DatatypeConstructorDecl("Error"));
    CVC::Sort astSort = slv.mkDatatypeSort(astSpec);
    const CVC::Datatype& astDT = astSort.getDatatype();
    CVC::Term astCon = astDT.getConstructorTerm("ast");
    CVC::Term isAST = astDT[0].getTesterTerm();
    CVC::Term isNone = astDT[1].getTesterTerm();

    Vt as;
    for (int i=0; i!= arity+1; i++)
        as.push_back(astDT["ast"].getSelectorTerm("a"+std::to_string(i)));
    CVC::Term noneterm = slv.mkTerm(CVC::APPLY_CONSTRUCTOR, astDT.getConstructorTerm("None"));
    CVC::Term errterm = slv.mkTerm(CVC::APPLY_CONSTRUCTOR, astDT.getConstructorTerm("Error"));

    // PATH
    CVC::DatatypeDecl pathSpec = slv.mkDatatypeDecl("Path");
    pathSpec.addConstructor(CVC::DatatypeConstructorDecl("Empty"));
    for (auto&& pp : paths){ for (auto&& p: pp){
            CVC::DatatypeConstructorDecl pcon("P"+join(p));
            pathSpec.addConstructor(pcon);
    }}
    CVC::Sort pathSort = slv.mkDatatypeSort(pathSpec);
    const CVC::Datatype& pathDT = pathSort.getDatatype();
    std::map<Vi,CVC::Term> pathcon;
    for (auto&& ps : paths) { for (auto&& p : ps) {
        pathcon[p] = slv.mkTerm(CVC::APPLY_CONSTRUCTOR, pathDT.getConstructorTerm("P"+join(p)));
    }}
    // RULE
    Vi rules; // +1/-1, +2/-2, etc. for each rule+direction
    CVC::DatatypeDecl ruleSpec = slv.mkDatatypeDecl("Rule");
    for (int i=1; i!=t.rules.size()+1; i++){ for (auto&& d : fr) {
            std::string name="R"+std::to_string(i)+ d;
            CVC::DatatypeConstructorDecl rcon(name);
            ruleSpec.addConstructor(rcon);
            rules.push_back(i*(d=='f' ? 1 : -1));
    }}
    CVC::Sort ruleSort = slv.mkDatatypeSort(ruleSpec);
    const CVC::Datatype& ruleDT = ruleSort.getDatatype();
    std::map<int,CVC::Term> rulecon;
    for (auto&& r:rules) {
        std::string name="R"+std::to_string(std::abs(r))+(r>0 ? 'f' : 'r');
        rulecon[r] = slv.mkTerm(CVC::APPLY_CONSTRUCTOR, ruleDT.getConstructorTerm(name));
    }
    // Convenience terms
    //-----------------------
    CVC::Term nInt = slv.mkVar(Int, "n");
    CVC::Term sInt = slv.mkVar(Int, "step");
    CVC::Term xTerm = slv.mkVar(astSort, "x");
    CVC::Term yTerm = slv.mkVar(astSort, "y");
    CVC::Term pTerm = slv.mkVar(pathSort, "p");
    CVC::Term rTerm = slv.mkVar(ruleSort, "r");
    CVC::Term Empty = slv.mkTerm(CVC::APPLY_CONSTRUCTOR, pathDT.getConstructorTerm("Empty"));
    CVC::Term node=astDT["ast"].getSelectorTerm("node");
    CVC::Term nodeX = slv.mkTerm(CVC::APPLY_SELECTOR, node, xTerm);
    CVC::Term noneX = slv.mkTerm(CVC::APPLY_TESTER, isNone, xTerm);
    CVC::Term astX = slv.mkTerm(CVC::APPLY_TESTER, isAST, xTerm);

    // Convenience functions
    //-----------------------

    // Things generated per-argument
    Vt xs;
    for (int i=0; i!= arity+1; i++)
        xs.push_back(slv.mkVar(astSort,"x"+std::to_string(i)));

    Vt As;
    for (int i=0; i!= arity+1; i++) {
        As.push_back(a_fun(slv,xTerm,astSort,Int,astDT,noneterm,noneX,i));
    }

    Vt asts;
    for (int i=0; i != arity+2; i++)
        asts.push_back(ast_fun(slv,xs,nInt,astSort,astDT,noneterm,i));

    CVC::Term Node = a_fun(slv,xTerm,astSort,Int,astDT,noneterm,noneX,-1);

    Vt repls;
    for (int i=0; i!= arity+1; i++){
        repls.push_back(repl_fun(slv, xTerm, yTerm, astSort, noneterm, nodeX, astCon, noneX, as, arity, i));
    }

    // Things generated per-path
    Vt pathConds{slv.mkTerm(CVC::EQUAL,pTerm,Empty)};
    Vt replaceAtThens{xTerm}, getAtThens{xTerm};
    Vt pathNumThens{slv.mkReal(-1)};
    std::map<Vi,CVC::Term> replPs;
    if (1) {
    for (auto&& ps : paths) { for (auto&& p : ps) {
        replPs[p] = replP_fun(slv,xTerm,yTerm,astSort,as,repls,p);
        pathConds.push_back(slv.mkTerm(CVC::EQUAL,pTerm,pathcon[p]));
        replaceAtThens.push_back(slv.mkTerm(CVC::APPLY_UF,replPs[p],xTerm,yTerm));
        pathNumThens.push_back(slv.mkReal("-1"+join(p)));
        getAtThens.push_back(pathterm(slv,xTerm,as,p));
    }}
    }
    CVC::Term replaceAt = slv.defineFun("replaceAt",{pTerm,xTerm,yTerm}, astSort,
                                        ITE(slv, pathConds, replaceAtThens, errterm));
    CVC::Term getAt = slv.defineFun("getAt",{xTerm,pTerm}, astSort,
                                    ITE(slv, pathConds, getAtThens, errterm));
    CVC::Term pathNum = slv.defineFun("pathNum",{pTerm}, Int,
                                       ITE(slv, pathConds, pathNumThens, slv.mkReal(0)));

    // Things generated per-rule
    Vt ruleConds, ruleThens;
    std::map<int,CVC::Term> pat,rterm;
    for (auto&& r : rules) {
        pat[r] = pat_fun(slv,xTerm,node,r, as,t,syms);
        rterm[r] = rterm_fun(slv,xTerm,sInt,astSort,r, t,asts,as,syms);
        ruleConds.push_back(slv.mkTerm(CVC::AND,slv.mkTerm(CVC::EQUAL,rTerm,rulecon[r]),
                                                slv.mkTerm(CVC::APPLY_UF,pat[r],xTerm)));
        ruleThens.push_back(slv.mkTerm(CVC::APPLY_UF,rterm[r],xTerm,sInt));
    }

    CVC::Term rewriteTop = slv.defineFun("rewriteTop",{rTerm,xTerm,sInt}, astSort,
                                        ITE(slv, ruleConds, ruleThens, errterm));

    // Overarching things
    if (1){
    CVC::Term rewrite = slv.defineFun("rewrite",{xTerm,rTerm,pTerm,sInt},astSort,
        slv.mkTerm(CVC::ITE,
                   astX,
                   slv.mkTerm(CVC::APPLY_UF,{
                              replaceAt,
                              pTerm,
                              xTerm,
                              slv.mkTerm(CVC::APPLY_UF, {
                                         rewriteTop,
                                         rTerm,
                                         slv.mkTerm(CVC::APPLY_UF,getAt,xTerm,pTerm),
                                         sInt})}),
                   errterm));

    // Finite rewrite functions
    Vt rs, ps, rws;
    for (int i=1; i < 8; i++) {
        rs.push_back(slv.mkConst(ruleSort, "r"+std::to_string(i)));
        ps.push_back(slv.mkConst(pathSort, "p"+std::to_string(i)));
        rws.push_back(rw_fun(slv,i,astSort,rewrite,rs,ps));
    }
}

    std::cout << "CONCRETE TERMS " << std::endl;
    if (0){
    // Create concrete terms
    //----------------------
    CVC::Term l7 = mkConst(slv, "l7", slv.mkTerm(CVC::APPLY_CONSTRUCTOR, {
        astDT.getConstructorTerm("ast"),slv.mkReal(7),
        noneterm, noneterm, noneterm, noneterm})); // leaf node with value 7

    CVC::Term l177 = mkConst(slv, "l177", slv.mkTerm(CVC::APPLY_UF, {asts[2], slv.mkReal(1), l7, l7}));
    CVC::Term l178 = mkConst(slv, "l178", slv.mkTerm(CVC::APPLY_UF, repls[1], l177,
        slv.mkTerm(CVC::APPLY_UF,asts[0],slv.mkReal(8)))) ;
    CVC::Term node_t = slv.mkTerm(CVC::APPLY_UF, Node, l7);

    // Assertions and print model
    //----------------------------
    CVC::Term assert1 = slv.mkTerm(CVC::GT, node_t, slv.mkReal(3));
    CVC::Term assert2 = slv.mkTerm(CVC::EQUAL, l7,
        slv.mkTerm(CVC::APPLY_UF, asts.at(0), slv.mkReal(7)));
    slv.assertFormula(assert1);
    slv.assertFormula(assert2);
    }
    CVC::Result res = slv.checkSat();
    std::cout << "Expect sat. CVC4: " << res << std::endl;

    if (res.isSat()){
        // Write model to file
        std::ofstream outfile;
        outfile.open("file.dat");
        slv.printModel(outfile);
        outfile.close();
    }
    return 0;
}

