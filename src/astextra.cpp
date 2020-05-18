#include "astextra.hpp"
#include <src/cvc4extra.hpp>


std::tuple<CVC::Sort,CVC::Sort,CVC::Sort> create_datatypes(
        CVC::Solver & slv,
        const Theory & t,
        const int & depth) {
    const int arity = max_arity(t);
    const Vvvi paths = all_paths(depth,arity);
    CVC::Sort Int = slv.getIntegerSort();
    const char fr[2]  = {'f','r'}; // Forward/reverse

    // AST
    CVC::DatatypeDecl astSpec = slv.mkDatatypeDecl("AST");
    astSpec.addConstructor(CVC::DatatypeConstructorDecl("Error"));
    astSpec.addConstructor(CVC::DatatypeConstructorDecl("None"));
    CVC::DatatypeConstructorDecl ast("ast");
    ast.addSelector("node", Int);
    for (int i=0; i<=arity;i++)
        ast.addSelectorSelf("a"+std::to_string(i));

    astSpec.addConstructor(ast);
    CVC::Sort astSort = slv.mkDatatypeSort(astSpec);

    // PATH
    CVC::DatatypeDecl pathSpec = slv.mkDatatypeDecl("Path");
    pathSpec.addConstructor(CVC::DatatypeConstructorDecl("Empty"));
    for (auto&& pp : paths){ for (auto&& p: pp){
            CVC::DatatypeConstructorDecl pcon("P"+join(p));
            pathSpec.addConstructor(pcon);
    }}
    CVC::Sort pathSort = slv.mkDatatypeSort(pathSpec);

    // RULE
    Vi rules; // +1/-1, +2/-2, etc. for each rule+direction
    CVC::DatatypeDecl ruleSpec = slv.mkDatatypeDecl("Rule");
    for (int i=1; i!=std::max(2,static_cast<int>(t.rules.size()+1)); i++){ for (auto&& d : fr) {
            std::string name="R"+std::to_string(i)+ d;
            CVC::DatatypeConstructorDecl rcon(name);
            ruleSpec.addConstructor(rcon);
            rules.push_back(i*(d=='f' ? 1 : -1));
    }}
    CVC::Sort ruleSort = slv.mkDatatypeSort(ruleSpec);

    return std::make_tuple(astSort,pathSort,ruleSort);
}

//-----------------------------------------------------------------

CVC::Term pat_fun(const CVC::Solver & slv,
                  const Theory & thry,
                  const CVC::Term & x,
                  const int & r,
                  const std::string & dir) {

    std::map<std::string,int> syms=symcode(thry);
    Rule rule = thry.rules.at(r-1);
    Expr term = dir=="f" ? rule.t1 : rule.t2;
    std::map<size_t,Vvi> groups = distinct(gethash(term));
    Vt andargs; // Represent term as list of constraints

    //std::cout << "ENTERING FOR LOOP for pattern " << term << std::endl;
    for (auto&& [k,v] : groups) {
        Vi rep = v.front();  // choose representative of equiv class
        //std::cout << "GETTING repE with rep size " << rep.size() << std::endl ;
        CVC::Term repE = subterm(slv, x, rep);
        Expr repX = subexpr(term, rep);
        const int sym = syms.at(repX.sym);

        // Node constraint on representative if not a variable
        if (repX.kind != VarNode){
            //std::cout << "Subexpr " << repX << "ADDING NODE CONSTRAINT " << sym << std::endl;
            CVC::Term andarg=slv.mkTerm(CVC::EQUAL,node(slv,repE),slv.mkReal(sym));
            //std::cout << andarg << "   IS " << slv.checkEntailed(andarg).isEntailed() << "  " << const_cast<CVC::Solver &>(slv).simplify(node(slv,repE))  << std::endl;
            andargs.push_back(andarg);
        }
        //std::cout << "ADDING EQ CONSTRAINT " << std::endl;

        // Eq constraint on all other members of eq class
        for (auto && e : v) {
            if (e!=rep){
                CVC::Term e_ = subterm(slv,x,e);
                andargs.push_back(slv.mkTerm(CVC::EQUAL,e_,repE));
            }
        }
    }

    CVC::Term ret=andargs.size()==1 ? andargs.front() : slv.mkTerm(CVC::AND,andargs);
    return ret;
}



CVC::Term rterm_fun(const CVC::Solver & slv,
                    const Theory & thry,
                    const CVC::Term & x,
                    const int & step,
                    const int & ruleind,
                    const std::string & dir) {

    Rule rule = thry.rules.at(ruleind-1);
    Expr src = dir=="f" ? rule.t1 : rule.t2;
    Expr tar = dir=="f" ? rule.t2 : rule.t1;

    // Get hash for each node
    std::map<Vi,size_t> srchash=gethash(src), tarhash=gethash(tar);
    std::map<size_t,Vi> srchashrev;
    for (auto && [k,v] : distinct(srchash)) srchashrev[k]=v.front();

    // Construct target in CVC4, making reference to source when possible

    //std::cout << "Calling construct with tar" << tar << " and src " << src << std::endl;
    CVC::Term res=construct(slv,x.getSort(),thry,tar,src,x,step);
    return res;
}



CVC::Term getAt(const CVC::Solver & slv,
                const CVC::Term & xTerm,
                const CVC::Term & pTerm,
                const Vvvi & paths) {
    Vt pathConds{test(slv, pTerm, "Empty")}, getAtThens{xTerm};
    for (auto&& ps : paths) { for (auto&& p : ps) {
        pathConds.push_back(test(slv,pTerm,"P"+join(p)));
        getAtThens.push_back(subterm(slv,xTerm,p));
    }}
    return ITE(slv, pathConds, getAtThens, unit(slv,xTerm.getSort(),"Error"));
}

CVC::Term replaceAt(const CVC::Solver & slv,
                    const CVC::Term & xTerm,
                    const CVC::Term & yTerm,
                    const CVC::Term & pTerm,
                   const Vvvi & paths) {
    Vt pathConds{test(slv,pTerm,"Empty")}, replaceAtThens{yTerm};
    for (auto&& ps : paths) { for (auto&& p : ps) {
        pathConds.push_back(test(slv,pTerm,"P"+join(p)));
        replaceAtThens.push_back(replP_fun(slv,xTerm,yTerm,p));
    }}
    return ITE(slv, pathConds, replaceAtThens, unit(slv,xTerm.getSort(),"Error"));
}

CVC::Term rewriteTop(const CVC::Solver & slv,
                    const CVC::Term & x,
                    const CVC::Term & rTerm,
                    const Theory & t,
                    const int & step) {
    Vt ruleConds,ruleThens;
    std::map<std::string,int> syms=symcode(t);
    for (int i=1;i<=t.rules.size();i++) { for (auto && ch : {"f","r"})   {
        CVC::Term req=test(slv,rTerm,"R"+std::to_string(i)+ch);
        //std::cout << "making pat" << i << ch << std::endl;
        CVC::Term rpat=pat_fun(slv,t,x,i,ch);
        //std::cout << "making term" <<  i << ch << std::endl;
        CVC::Term rt=rterm_fun(slv,t, x, step,i,ch);
        ruleConds.push_back(slv.mkTerm(CVC::AND,req,rpat));
        ruleThens.push_back(rt);
    }}

    return ITE(slv, ruleConds, ruleThens, unit(slv,x.getSort(),"Error"));
}


CVC::Term rewrite(const CVC::Solver & slv,
                  const Theory & t,
                  const CVC::Term & x,
                  const CVC::Term & r,
                  const CVC::Term & p,
                  const int & step,
                  const int & depth
                  ) {
    Vvvi paths = all_paths(depth,arity(x.getSort())-1);

    CVC::Term presub = getAt(slv,x,p,paths);
    //std::cout << "presub" << std::endl;
    CVC::Term subbed = rewriteTop(slv,  presub, r, t, step);
    //std::cout << "subbed" << std::endl;
    CVC::Term ret=replaceAt(slv, x, subbed, p, paths);
    //std::cout << "ret" << std::endl;

    return ret;
}

CVC::Term assert_rewrite(const CVC::Solver & slv,
             const CVC::Sort & astSort,
             const CVC::Sort & pathSort,
             const CVC::Sort & ruleSort,
             const Theory & t,
             const CVC::Term & t1,
             const CVC::Term & t2,
             const int & steps
             ) {
    //std::cout << c1 << "\n\n" << c2 << std::endl;
    Vt ps,rs,xs{t1};
    for (int i=0;i<steps;i++){
        //std::cout <<  "REWRITE STEP " << i << std::endl;
        ps.push_back(slv.mkConst(pathSort,"p"+std::to_string(i)));
        rs.push_back(slv.mkConst(ruleSort,"r"+std::to_string(i)));
        xs.push_back(rewrite(slv, t, xs.at(i),rs.at(i),ps.at(i),i,steps));
    }

    return mkConst(slv, "rw", slv.mkTerm(CVC::EQUAL,xs.back(),t2));
}