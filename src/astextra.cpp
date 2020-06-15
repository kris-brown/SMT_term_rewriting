#include "astextra.hpp"
#include "cvc4extra.hpp"


std::tuple<smt::Sort,smt::Sort,smt::Sort> create_datatypes(
        smt::SmtSolver & slv,
        const Theory & t,
        const int & depth) {
    const int arity = t.max_arity();
    const Vvvi paths = all_paths(depth,arity);
    smt::Sort Int = slv->make_sort(smt::INT);
    const char fr[2]  = {'f','r'}; // Forward/reverse

    // AST
    smt::DatatypeDecl astSpec = slv->make_datatype_decl("AST");
    slv->add_constructor(astSpec,slv->make_datatype_constructor_decl("Error"));
    slv->add_constructor(astSpec,slv->make_datatype_constructor_decl("None"));
    smt::DatatypeConstructorDecl ast = slv->make_datatype_constructor_decl("ast");
    slv->add_selector(ast, "node", Int);
    for (int i=0; i<=arity;i++)
        slv->add_selector_self(ast,"a"+std::to_string(i));

    slv->add_constructor(astSpec, ast);
    smt::Sort astSort = slv->make_sort(astSpec);

    // PATH
    smt::DatatypeDecl pathSpec = slv->make_datatype_decl("Path");
    slv->add_constructor(pathSpec,slv->make_datatype_constructor_decl("Empty"));
    for (auto&& pp : paths){ for (auto&& p: pp){
            slv->add_constructor(pathSpec,slv->make_datatype_constructor_decl("P"+join(p)));
    }}
    smt::Sort pathSort = slv->make_sort(pathSpec);

    // RULE
    smt::DatatypeDecl ruleSpec = slv->make_datatype_decl("Rule");
    for (int i=1; i!=std::max(2,static_cast<int>(t.rules.size()+1)); i++){ for (auto&& d : fr) {
            std::string name="R"+std::to_string(i)+ d;
            slv->add_constructor(ruleSpec,slv->make_datatype_constructor_decl(name));
    }}
    smt::Sort ruleSort = slv->make_sort(ruleSpec);

    return std::make_tuple(astSort,pathSort,ruleSort);
}

//-----------------------------------------------------------------

smt::Term pat_fun(const smt::SmtSolver & slv,
                  const Theory & thry,
                  const smt::Term & x,
                  const int & r,
                  const std::string & dir) {

    std::map<std::string,int> syms=thry.symcode();
    Rule rule = thry.rules.at(r-1);
    Expr term = dir=="f" ? rule.t1 : rule.t2;
    std::map<size_t,Vvi> groups = Expr::distinct(term.gethash());
    Vt andargs; // Represent term as list of constraints

    //std::cout << "ENTERING FOR LOOP for pattern " << term << std::endl;
    for (auto&& [k,v] : groups) {
        Vi rep = v.front();  // choose representative of equiv class
        //std::cout << "GETTING repE with rep size " << rep.size() << std::endl ;
        smt::Term repE = subterm(slv, x, rep);
        Expr repX = term.subexpr(rep);
        const int sym = syms.at(repX.sym);

        // Node+leaf constraint on representative if not a variable
        if (repX.kind != Expr::VarNode){
            smt::Term andarg=slv->make_term(smt::Equal,node(slv,repE),slv->make_term(sym, slv->make_sort(smt::INT)));
            andargs.push_back(andarg);
            for (int i=repX.args.size();i!=arity(x->get_sort());i++){
                andargs.push_back(test(slv,getarg(slv,repE,i),"None")); }
        }


        // Eq constraint on all other members of eq class
        for (auto && e : v) {
            if (e!=rep){
                smt::Term e_ = subterm(slv,x,e);
                andargs.push_back(slv->make_term(smt::Equal,e_,repE));
            }
        }
    }

    smt::Term ret = slv->make_term(smt::And,andargs);
    return ret;
}



smt::Term rterm_fun(const smt::SmtSolver & slv,
                    const Theory & thry,
                    const smt::Term & x,
                    const smt::Term & step,
                    const int & ruleind,
                    const std::string & dir) {

    Rule rule = thry.rules.at(ruleind-1);
    Expr src = dir=="f" ? rule.t1 : rule.t2;
    Expr tar = dir=="f" ? rule.t2 : rule.t1;

    // Get hash for each node
    std::map<Vi,size_t> srchash=src.gethash(), tarhash=tar.gethash();
    std::map<size_t,Vi> srchashrev;
    for (auto && [k,v] : Expr::distinct(srchash)) srchashrev[k]=v.front();

    // Construct target in CVC4, making reference to source when possible

    //std::cout << "Calling construct with tar" << tar << " and src " << src << std::endl;
    smt::Term res=construct(slv,x->get_sort(),thry,tar,src,x,step);
    return res;
}

smt::Term getAt(const smt::SmtSolver & slv,
                const smt::Term & xTerm,
                const smt::Term & pTerm,
                const Vvvi & paths) {
    smt::Term err=unit(slv,xTerm->get_sort(),"Error");
    Vt pathConds{ntest(slv,xTerm,"ast"),
                 test(slv, pTerm, "Empty")};
    Vt getAtThens{err, xTerm};
    for (auto&& ps : paths) { for (auto&& p : ps) {
        pathConds.push_back(test(slv,pTerm,"P"+join(p)));
        getAtThens.push_back(subterm(slv,xTerm,p));
    }}
    return ITE(slv, pathConds, getAtThens, err);
}

smt::Term replaceAt(const smt::SmtSolver & slv,
                    const smt::Term & xTerm,
                    const smt::Term & yTerm,
                    const smt::Term & pTerm,
                   const Vvvi & paths) {
    smt::Term err=unit(slv,xTerm->get_sort(),"Error");

    Vt pathConds{ntest(slv,xTerm,"ast"), test(slv,pTerm,"Empty")};
    Vt replaceAtThens{err, yTerm};
    for (auto&& ps : paths) { for (auto&& p : ps) {
        pathConds.push_back(test(slv,pTerm,"P"+join(p)));
        replaceAtThens.push_back(replP_fun(slv,xTerm,yTerm,p));
    }}
    return ITE(slv, pathConds, replaceAtThens, err);
}

smt::Term rewriteTop(const smt::SmtSolver & slv,
                    const smt::Term & x,
                    const smt::Term & rTerm,
                    const Theory & t,
                    const smt::Term & step) {
    Vt ruleConds{ntest(slv,x,"ast")}, ruleThens{unit(slv,x->get_sort(),"Error")};
    std::map<std::string,int> syms=t.symcode();
    for (int i=1;i<=t.rules.size();i++) { for (auto && ch : {"f","r"})   {
        smt::Term req=test(slv,rTerm,"R"+std::to_string(i)+ch);
        //std::cout << "making pat" << i << ch << std::endl;
        smt::Term rpat=pat_fun(slv,t,x,i,ch);
        //std::cout << "making term" <<  i << ch << std::endl;
        smt::Term rt=rterm_fun(slv,t, x, step,i,ch);
        ruleConds.push_back(slv->make_term(smt::And,req,rpat));
        ruleThens.push_back(rt);
    }}

    return ITE(slv, ruleConds, ruleThens, unit(slv,x->get_sort(),"Error"));
}


smt::Term rewrite(const smt::SmtSolver & slv,
                  const Theory & t,
                  const smt::Term & x,
                  const smt::Term & r,
                  const smt::Term & p,
                  const smt::Term & step,
                  const int & depth
                  ) {
    Vvvi paths = all_paths(depth,t.max_arity());

    smt::Term presub = getAt(slv,x,p,paths);
    smt::Term subbed = rewriteTop(slv,  presub, r, t, step);
    smt::Term ret=replaceAt(slv, x, subbed, p, paths);

    return slv->make_term(smt::Ite,ntest(slv,x,"ast"),
                      unit(slv,x->get_sort(),"Error"), ret);
}

