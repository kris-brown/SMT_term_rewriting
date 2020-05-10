#include<stdexcept>
#include <src/astextra.hpp>



// Cartesian product of vector of vectors, taken from: https://stackoverflow.com/a/5279601
void cart_product(
    Vvi& rvvi,  // final result
    Vi&  rvi,   // current result
    Vvi::const_iterator me, // current input
    Vvi::const_iterator end) // final input
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
// Path vector generation: we want (arity ^ depth) number of paths
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
// All paths of length 1, 2, ... depth
Vvvi all_paths(const int&  depth, const int& arity) {
        Vvvi res;
        for (int i=1; i <= depth; i++) {
            res.push_back(paths_n(i, arity));
        }
        return res;
}



//----------------------------------------------------------------------------------------------------------------
// HELPER FUNCTIONS FOR C++ API

// e.g. ast3 : (INT, AST, AST, AST) -> AST
// = (LAMBDA(n:INT, x0:AST, x1:AST, x2:AST): ast(n, x0, x1, x2, None));
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

// Safe access to the selectors of the AST constructor
// e.g.    A2 : T -> T   = LAMBDA (x:T): IF x = None THEN None ELSE a2(x) ENDIF;
// Also: Node : T -> INT = LAMBDA (x:T): IF x = None THEN 0    ELSE node(x) ENDIF;
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
// Replace a top-level argument of a term
// replace2 : (T,T) -> T = LAMBDA (x,y: T):ast(node(x),a0(x),a1(x),y,a3(x));
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
/* Replace a specific subnode of a term
replaceP321 : (T, T) -> T = LAMBDA(x,y:T): replace3(x, replace2(a3(x), replace1(a2(a3(x)), y)));
*/
CVC::Term replP_fun(CVC::Solver & slv,
                const CVC::Term & x,
                const CVC::Term & y,
                const CVC::Sort & astSort,
                const Vt & as,
                const Vt & reps,
                const Vi & p) {
    CVC::Term arg=y; // TBD
    for (int i=0;i != p.size(); i++) {
        CVC::Term subx=x; // JUST TEMPORARILY -- TODO
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
        x = slv.mkTerm(CVC::APPLY_SELECTOR, selectors[i], x);
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


// Render a (sub)term in some argument term (make reference to subexpressions when possible)
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
    std::set<std::string> symx,symy,diff;
    addx(symx,x); addx(symy,y,VarNode);

    std::map<std::string,int> res;
    int i = 1;
    for (auto && e : symx)
    if (symy.find(e)==symy.end())
        res[e]=i++;
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
             {rewrite, xs.back(), rs[i], ps[i], slv.mkReal(i+1)});
        xs.push_back(x);
    }
    CVC::Term ret = slv.defineFun("rewrite"+std::to_string(imax),
                                  {xs[0]},astSort,xs.back());
    return ret;
}
