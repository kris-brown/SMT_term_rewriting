#include "astextra.hpp"
#include <src/cvc4extra.hpp>

void cart_product(
    Vvi& rvvi,
    Vi&  rvi,
    Vvi::const_iterator me,
    Vvi::const_iterator end)
{
    if(me == end) {
        rvvi.push_back(rvi);
        return;
    }
    const Vi& mevi = *me;
    for(Vi::const_iterator it = mevi.begin();
        it != mevi.end();
        it++) {
        rvi.push_back(*it);  // add ME
        cart_product(rvvi, rvi, me+1, end); //add "d, e, f"
        rvi.pop_back(); // clean ME off for next round
    }
}
//----------------------------------------------------------------------------------------------------------------
Vvi paths_n(const int&  depth, const int& arity) {
    Vi arg_inds;
    for (int i=0; i <= arity; i++)
        { arg_inds.push_back(i);}
    const Vvi input(depth, arg_inds);
    Vvi output;
    Vi out_tmp;
    cart_product(output, out_tmp, input.begin(), input.end());
    return output;
}

Vvvi all_paths(const int&  depth, const int& arity) {
        Vvvi res;
        for (int i=1; i <= depth; i++) {
            res.push_back(paths_n(i, arity));
        }
        return res;
}



CDTuple create_datatypes(CVC::Solver & slv,
                      const Theory & t,
                      const int & depth) {
    const int arity=max_arity(t);
    const Vvvi paths = all_paths(depth,arity);
    CVC::Sort Int = slv.getIntegerSort();
    const char fr[2]  = {'f','r'}; // Forward/reverse

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
    CVC::Term node=astDT["ast"].getSelectorTerm("node");
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
    CVC::Term Empty = slv.mkTerm(CVC::APPLY_CONSTRUCTOR, pathDT.getConstructorTerm("Empty"));

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
    CVC::Term nodeX = slv.mkTerm(CVC::APPLY_SELECTOR, node, xTerm);
    CVC::Term noneX = slv.mkTerm(CVC::APPLY_TESTER, isNone, xTerm);
    CVC::Term astX = slv.mkTerm(CVC::APPLY_TESTER, isAST, xTerm);

    return std::make_tuple(
        Int,astSort,pathSort,ruleSort,
        astDT,pathDT,ruleDT,
        astCon, node,isNone, isAST, noneterm, errterm,
        Empty, nInt,sInt,xTerm,yTerm,pTerm,rTerm,nodeX,noneX,astX, as, rules, pathcon, rulecon);
}

std::tuple<Vt,Vt,CVC::Term> arity_funcs(CVC::Solver & slv,
                         const int & arity,
                         const CVC::Sort & Int,
                         const CVC::Sort & astSort,
                         const CVC::Datatype & astDT,
                         const CVC::Term & astCon,
                         const CVC::Term & nInt,
                         const CVC::Term & xTerm,
                         const CVC::Term & yTerm,
                         const CVC::Term & noneterm,
                         const CVC::Term & noneX,
                         const CVC::Term & nodeX,
                         const Vt & as
    ) {
    Vt xs;
    for (int i=0; i!= arity+1; i++)
        xs.push_back(slv.mkVar(astSort,"x"+std::to_string(i)));

    // Vt As; CURRENTLY NOT NEEDED
    // for (int i=0; i!= arity+1; i++) {
    //     As.push_back(a_fun(slv,xTerm,astSort,Int,astDT,noneterm,noneX,i));}

    Vt asts;
    for (int i=0; i != arity+2; i++)
        asts.push_back(ast_fun(slv,xs,nInt,astSort,astDT,noneterm,i));

    CVC::Term Node = a_fun(slv,xTerm,astSort,Int,astDT,noneterm,noneX,-1);

    Vt repls;
    for (int i=0; i!= arity+1; i++)
        repls.push_back(repl_fun(slv, xTerm, yTerm, astSort, noneterm, nodeX, astCon, noneX, as, arity, i));
    return std::make_tuple(repls, asts, Node);
}

std::tuple<CVC::Term,CVC::Term> path_funcs(
    CVC::Solver & slv,
    const CVC::Sort &  astSort,
    const CVC::Sort &  Int,
    const CVC::Term & xTerm,
    const CVC::Term & yTerm,
    const CVC::Term & pTerm,
    const CVC::Term & errterm,
    const CVC::Term & Empty,
    const Vt & as,
    const Vt & repls,
    const Vvvi & paths,
    const std::map<Vi,CVC::Term> & pathcon) {
    Vt pathConds{slv.mkTerm(CVC::EQUAL,pTerm,Empty)};
    Vt replaceAtThens{xTerm}, getAtThens{xTerm};
    Vt pathNumThens{slv.mkReal(-1)};
    std::map<Vi,CVC::Term> replPs;
    if (1) {
    for (auto&& ps : paths) { for (auto&& p : ps) {
        replPs[p] = replP_fun(slv,xTerm,yTerm,astSort,as,repls,p);
        pathConds.push_back(slv.mkTerm(CVC::EQUAL,pTerm,pathcon.at(p)));
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
    return std::make_tuple(getAt,replaceAt);
}

std::tuple<Tmap, Tmap, CVC::Term> rule_funcs(
    CVC::Solver & slv,
    const Theory & t,
    const CVC::Sort &  astSort,
    const CVC::Sort &  Int,
    const CVC::Term &  xTerm,
    const CVC::Term &  rTerm,
    const CVC::Term &  errterm,
    const CVC::Term &  sInt,
    const CVC::Term &  node,
    const std::map<int,CVC::Term> & rulecon,
    const Vt &  as,
    const Vt &  asts,
    const Vi & rules) {
    Vt ruleConds, ruleThens;
    Tmap pat,rterm;
    std::map<std::string,int> syms = symcode(t);

    for (auto&& r : rules) {
        pat[r] = pat_fun(slv,xTerm,node,r, as,t,syms);
        rterm[r] = rterm_fun(slv,xTerm,sInt,astSort,r, t,asts,as,syms);
        ruleConds.push_back(slv.mkTerm(CVC::AND,slv.mkTerm(CVC::EQUAL,rTerm,rulecon.at(r)),
                                                slv.mkTerm(CVC::APPLY_UF,pat[r],xTerm)));
        ruleThens.push_back(slv.mkTerm(CVC::APPLY_UF,rterm[r],xTerm,sInt));
    }

    CVC::Term rewriteTop = slv.defineFun("rewriteTop",{rTerm,xTerm,sInt}, astSort,
                                        ITE(slv, ruleConds, ruleThens, errterm));
    return std::make_tuple(pat,rterm,rewriteTop);
}

std::tuple<CVC::Term,Vt> rewrite_funcs(
    CVC::Solver & slv,
    const CVC::Sort & astSort,
    const CVC::Sort & ruleSort,
    const CVC::Sort & pathSort,
    const CVC::Term & xTerm,
    const CVC::Term & rTerm,
    const CVC::Term & pTerm,
    const CVC::Term & sInt,
    const CVC::Term & astX,
    const CVC::Term & errterm,
    const CVC::Term & replaceAt,
    const CVC::Term & rewriteTop,
    const CVC::Term & getAt) {
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
    return std::make_tuple(rewrite,rws);
}

STuple setup(CVC::Solver & slv,
             const Theory & t,
             const int & depth,
             const bool incl_rewrite,
             const bool incl_rule,
             const bool incl_path,
             const bool incl_arity) {
    const int arity=max_arity(t);
    const Vvvi paths = all_paths(depth,arity);

    // Declare datatypes
    CVC::Sort Int,astSort,pathSort,ruleSort;
    CVC::Datatype astDT,pathDT,ruleDT;
    CVC::Term astCon, node, isNone, isAST, noneterm, errterm, Empty, nInt,sInt,xTerm,yTerm,pTerm,rTerm,nodeX,noneX,astX;
    Vt as; Vi rules;
    std::map<Vi,CVC::Term> pathcon; Tmap rulecon;
    std::tie (
        Int,astSort,pathSort,ruleSort,
        astDT,pathDT,ruleDT,
        astCon, node, isNone, isAST, noneterm, errterm,
        Empty, nInt,sInt,xTerm,yTerm,pTerm,rTerm,nodeX,noneX,astX, as, rules, pathcon, rulecon) = create_datatypes(slv,t,depth);

    // Things generated per-argument
    Vt repls, asts;
    CVC::Term Node;
    if (incl_arity)
        std::tie (repls, asts, Node) = arity_funcs(slv,arity,Int,astSort,astDT,
            astCon,nInt,xTerm,yTerm,noneterm,noneX,nodeX, as);

    // Things generated per-path
    CVC::Term getAt, replaceAt;
    if (incl_path)
        std::tie (getAt, replaceAt) = path_funcs(slv, astSort, Int, xTerm, yTerm, pTerm,
            errterm, Empty, as, repls, paths, pathcon);

    // Things generated per-rule
    Tmap rpat, rterm;
    CVC::Term rewriteTop;
    if (incl_rule)
        std::tie (rpat,rterm,rewriteTop) = rule_funcs(slv,t,astSort,Int,xTerm,rTerm,
            errterm,sInt,node,rulecon,as,asts,rules);


    // Overarching rewrite infrastructure
    Vt rws;
    CVC::Term rewrite;
    if (incl_rewrite)
        std::tie (rewrite, rws) = rewrite_funcs(slv,astSort,ruleSort,pathSort,xTerm,rTerm,
            pTerm,sInt,astX,errterm,replaceAt,rewriteTop,getAt);

    // Things of potential interest for other functions to use
    return std::make_tuple(Int,astSort,pathSort,ruleSort,
        astDT, pathDT, ruleDT,
        astCon, node, isNone, isAST, noneterm, errterm, Empty,
        as,rules,repls,asts);
}
//-----------------------------------------------------------------

CVC::Term ast_fun(CVC::Solver & slv,
                  const Vt & xs,
                  const CVC::Term & nInt,
                  const CVC::Sort & astSort,
                  const CVC::Datatype & astDT,
                  const CVC::Term & noneterm,
                  const int & i) {
    Vt args{nInt};
    args.insert( args.end(), xs.begin(), xs.begin()+i );
    Vt nones(xs.size()-i,noneterm);
    Vt args2{astDT.getConstructorTerm("ast")};
    args2.insert( args2.end(), args.begin(), args.end());
    args2.insert( args2.end(), nones.begin(), nones.end());
    CVC::Term ret = slv.defineFun("ast"+std::to_string(i),args,astSort,
        slv.mkTerm(CVC::APPLY_CONSTRUCTOR,args2));
    return ret;
}

CVC::Term a_fun(CVC::Solver & slv,
                const CVC::Term & x,
                const CVC::Sort & astSort,
                const CVC::Sort & intSort,
                const CVC::Datatype & astDT,
                const CVC::Term & noneterm,
                const CVC::Term & noneX,
                const int & i) {
    Vt args{x};
    bool is_node = i == -1;
    std::string sel = is_node ? "node" : "a"+std::to_string(i);
    CVC::Term E = slv.mkTerm(CVC::APPLY_SELECTOR, astDT["ast"].getSelectorTerm(sel), x);
    CVC::Term T = is_node ? slv.mkReal(0) : noneterm;
    CVC::Sort s = is_node ? intSort : astSort;
    std::string name = is_node ? "Node" : "A"+std::to_string(i);
    CVC::Term ite = slv.mkTerm(CVC::ITE,noneX,T,E);
    CVC::Term ret = slv.defineFun(name,args,s,ite);
    return ret;
}

CVC::Term repl_fun(CVC::Solver & slv,
                const CVC::Term & x,
                const CVC::Term & y,
                const CVC::Sort & astSort,
                const CVC::Term & noneterm,
                const CVC::Term & nodeX,
                const CVC::Term & astCon,
                const CVC::Term & noneX,
                const Vt & as,
                const int & arity,
                const int & i) {
    Vt targs{astCon, nodeX};
    for (int j=0; j<=arity;j++){
        if (j==i) targs.push_back(y);
        else {
            targs.push_back(slv.mkTerm(CVC::APPLY_SELECTOR,as.at(j), x));}
    }

    CVC::Term E = slv.mkTerm(CVC::APPLY_CONSTRUCTOR, targs);
    CVC::Term ite = slv.mkTerm(CVC::ITE,noneX,noneterm,E);
    std::string name="replace"+std::to_string(i);
    CVC::Term ret = slv.defineFun(name,{x,y},astSort,ite);

    return ret;
}

CVC::Term replP_fun(CVC::Solver & slv,
                const CVC::Term & x,
                const CVC::Term & y,
                const CVC::Sort & astSort,
                const Vt & as,
                const Vt & reps,
                const Vi & p) {
    CVC::Term arg=y;
    for (int i=0;i != p.size(); i++) {
        CVC::Term subx=x;
        for (int j=p.size()-1;i!=j;j--)
            subx = slv.mkTerm(CVC::APPLY_SELECTOR,as.at(p.at(j)),subx);
        arg=slv.mkTerm(CVC::APPLY_UF,reps.at(p.at(i)),subx,arg);
    }
    CVC::Term t=slv.mkTerm(CVC::APPLY_UF,reps.at(p.at(0)),x,arg);
    std::string name="replaceP"+join(p);
    CVC::Term ret = slv.defineFun(name,{x,y},astSort,t);
    return ret;
}

CVC::Term pathterm(const CVC::Solver & slv,
                   const CVC::Term & root,
                   const Vt & selectors,
                   const Vi & pth) {
    CVC::Term x = root;
    for (auto&& i : pth)
        x = slv.mkTerm(CVC::APPLY_SELECTOR, selectors.at(i), x);
    return x;
}


CVC::Term pat_fun(const CVC::Solver & slv,
                  const CVC::Term & x,
                  const CVC::Term & node,
                  const int & r,
                  const Vt & selectors,
                  const Theory & thry,
                  const std::map<std::string,int> & symcode) {
    std::string name="r"+std::to_string(std::abs(r))+(r>0 ? 'f' : 'r')+"pat";

    if (thry.rules.empty())
        return slv.defineFun(name,{x},slv.getBooleanSort(),slv.mkTrue());

    Rule rule = thry.rules.at(std::abs(r)-1);
    Expr term = r>0 ? rule.t1 : rule.t2;
    std::map<size_t,Vvi> groups = distinct(gethash(term));
    Vt andargs; // Represent term as list of constraints

    for (auto&& [k,v] : groups) {
        Vi rep = v.front();  // choose representative of equiv class
        CVC::Term repE = pathterm(slv,x,selectors,rep);
        const int sym = symcode.at(subexpr(term,rep).sym);
        // Node constraint on representative
        CVC::Term repnode=slv.mkTerm(CVC::APPLY_SELECTOR,node,repE);
        andargs.push_back(slv.mkTerm(CVC::EQUAL,repnode,slv.mkReal(sym)));
        // Eq constraint on all other members of eq class
        for (auto && e : v) {
            if (e!=rep){
                CVC::Term e_ = pathterm(slv,x,selectors,e);
                andargs.push_back(slv.mkTerm(CVC::EQUAL,e_,repE));
            }
        }
    }

    CVC::Term ands=andargs.size()==1 ? andargs.front() : slv.mkTerm(CVC::AND,andargs);
    CVC::Term ret=slv.defineFun(name,{x},slv.getBooleanSort(),ands);
    return ret;
}


CVC::Term construct(const CVC::Solver & slv,
                    const Expr & tar,
                    const Vi & currpth,
                    const std::map<size_t,Vi> & srchsh,
                    const std::map<Vi,size_t> & tarhsh,
                    const CVC::Term & x,
                    const CVC::Term & step,
                    const Vt & asts,
                    const Vt & selectors,
                    const std::map<std::string,int> freevar,
                    const std::map<std::string,int> & symcode) {
    size_t currhsh = tarhsh.at(currpth);
    if (srchsh.find(currhsh)!=srchsh.end())
        return pathterm(slv,x,selectors,srchsh.at(currhsh));
    else {
        CVC::Term node;
        if (freevar.find(tar.sym)!=freevar.end()) {
            node=slv.mkTerm(CVC::MINUS,slv.mkTerm(CVC::MULT,slv.mkReal(-10),step),
                                       slv.mkReal(freevar.at(tar.sym)));
        }
        else {
            node=slv.mkReal(symcode.at(tar.sym));
         }
        Vt args{asts.at(tar.args.size()),node};
        for (int i=0;i!=tar.args.size();i++) {
            Vi newpth;
            for (auto && j : currpth) newpth.push_back(j);
            newpth.push_back(i);
            args.push_back(construct(slv,tar.args.at(i),newpth,srchsh,tarhsh,x,
                step,asts,selectors,freevar,symcode));
        }
        return slv.mkTerm(CVC::APPLY_UF,args);
    }
}

std::map<std::string,int> mk_freevar(Expr x, Expr y) {
    std::set<std::string> symx,symy;
    addx(symx,x,VarNode); addx(symy,y,VarNode);

    std::map<std::string,int> res;
    int i = 1;
    for (auto && e : symx){
    if (symy.find(e)==symy.end())
        res[e]=i++;}
    return res;
}


CVC::Term rterm_fun(const CVC::Solver & slv,
                    const CVC::Term & x,
                    const CVC::Term & s,
                    const CVC::Sort & astSort,
                    const int & r,
                    const Theory & thry,
                    const Vt & asts,
                    const Vt & selectors,
                    std::map<std::string,int> & symcode) {
    std::string name="r"+std::to_string(std::abs(r))+(r>0 ? 'f' : 'r')+"term";
    if (thry.rules.empty())
     return slv.defineFun(name,{x,s},astSort,x);

    Rule rule = thry.rules.at(std::abs(r)-1);
    Expr src = r>0 ? rule.t1 : rule.t2;
    Expr tar = r>0 ? rule.t2 : rule.t1;

    // Get hash for each node
    std::map<Vi,size_t> srchash=gethash(src), tarhash=gethash(tar);
    std::map<size_t,Vi> srchashrev;
    for (auto && [k,v] : distinct(srchash)) srchashrev[k]=v.front();

    // Construct target in CVC4, making reference to source when possible
    CVC::Term res=construct(slv,tar,{},srchashrev,tarhash,x,s,asts,selectors, mk_freevar(tar,src),symcode);
    CVC::Term ret=slv.defineFun(name,{x,s},astSort,res);
    return ret;
}

CVC::Term rw_fun(const CVC::Solver & slv,
                 const int & imax,
                 const CVC::Sort & astSort,
                 const CVC::Term & rewrite,
                 const Vt & rs,
                 const Vt & ps) {
    Vt xs{slv.mkVar(astSort,"x0")};
    for (int i=0; i<imax; i++){
        CVC::Term x=slv.mkTerm(CVC::APPLY_UF,
             {rewrite, xs.back(), rs.at(i), ps.at(i), slv.mkReal(i+1)});
        xs.push_back(x);
    }
    CVC::Term ret = slv.defineFun("rewrite"+std::to_string(imax),
                                  {xs[0]},astSort,xs.back());
    return ret;
}
