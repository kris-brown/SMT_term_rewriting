#include "astextra_basic.hpp"

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

CVC::Term unit(const CVC::Solver & slv,
               const CVC::Sort & srt,
               const std::string &name) {
    return slv.mkTerm(CVC::APPLY_CONSTRUCTOR,
                      srt.getDatatype().getConstructorTerm(name));
}

int arity(const CVC::Sort & astSort) {
    return astSort.getDatatype().getConstructor("ast").getNumSelectors() - 1;
}

CVC::Term node(const CVC::Solver & slv,
              const CVC::Term & x) {
    CVC::Term s=x.getSort().getDatatype().getConstructor("ast").getSelectorTerm("node");
    return slv.mkTerm(CVC::APPLY_SELECTOR, s, x);
}

CVC::Term getarg(const CVC::Solver & slv,
                const CVC::Term & x,
                const int & i) {
    CVC::Term s=x.getSort().getDatatype().getConstructor("ast"
                ).getSelectorTerm("a"+std::to_string(i));
    return slv.mkTerm(CVC::APPLY_SELECTOR, s, x);
}


CVC::Term ast(const CVC::Solver & slv,
              const CVC::Sort & astSort,
              const CVC::Term & n,
              const Vt & xs
              ) {
    Vt args{astSort.getDatatype().getConstructorTerm("ast"), n};
    for (auto && x: xs) args.push_back(x);
    CVC::Term nt = unit(slv,astSort,"None");
    for (int i=0; i!=arity(astSort)-xs.size(); i++)
        args.push_back(nt);
    CVC::Term ret=slv.mkTerm(CVC::APPLY_CONSTRUCTOR,args);
    return ret;
}

CVC::Term replace(const CVC::Solver & slv,
                  const int & arg,
                  const CVC::Term & x,
                  const CVC::Term & y) {
    Vt args;
    CVC::Sort astSort=x.getSort();
    for (int j=0; j!=arity(astSort);j++){
        if (j==arg) args.push_back(y);
        else {
            args.push_back(getarg(slv,x,j));}
    }
    return ast(slv,astSort,node(slv,x),args);
}

CVC::Term replP_fun(const CVC::Solver & slv,
                    const CVC::Term & x,
                    const CVC::Term & y,
                    const Vi & p) {
    CVC::Term arg=y;
    for (int i=0;i != p.size(); i++) {
        CVC::Term subx=x;
        for (int j=p.size()-1;i!=j;j--)
            subx = getarg(slv,subx,p.at(j));
        arg=replace(slv,p.at(i),subx,arg);
    }
    return arg;
}

CVC::Term subterm(const CVC::Solver & slv,
                   const CVC::Term & root,
                   const Vi & pth) {
    CVC::Term x = root;
    assert(x.getSort().toString()=="AST");
    //std::cout << "\tin subterm with pth len " << pth.size() << " and got sort " << std::endl;

    CVC::DatatypeConstructor s = x.getSort().getDatatype().getConstructor("ast");
    CVC::Term sel;
    //std::cout << "\tgot DT " << std::endl;
    for (auto&& i : pth){
        sel = s.getSelectorTerm("a"+std::to_string(i));
        x = slv.mkTerm(CVC::APPLY_SELECTOR, sel, x);}
    return x;
}

int strhash(const std::string & s) {
    return 100 + static_cast<int>(std::hash<std::string>()(s)) % 1000;
}

CVC::Term test(const CVC::Solver & slv,
               const CVC::Term & x,
               const std::string & s) {
    return slv.mkTerm(CVC::APPLY_TESTER,x.getSort().getDatatype().getConstructor(s).getTesterTerm(),x);
}


// Build a term in reference to another term, e.g. y from x
// x=(1+(2+3)) and y=((2+3)+2) then we get y=(x.1 + x.1.0)
CVC::Term construct(const CVC::Solver & slv,
                    const CVC::Sort & astSort,
                    const Theory & t,
                    const Expr & tar,
                    const Expr & src,
                    const CVC::Term & src_t,
                    const int & step) {
    std::map<Vi,size_t> srchash, tarh=gethash(tar);
    std::map<std::string,int> fv;
    std::map<size_t,Vi> srch;
    if (src.sym!="?") {
        srchash=gethash(src);
        fv=freevar(tar,src);}
    for (auto && [k,v] : distinct(srchash)) srch[k]=v.front(); //reverse
    std::map<std::string,int> syms = symcode(t);

    return constructRec(slv,astSort,tar,{},src_t,srch,tarh,step,fv,syms);
}

CVC::Term constructRec(const CVC::Solver & slv,
                       const CVC::Sort & astSort,
                       const Expr & tar,
                       const Vi & currpth,
                       const CVC::Term & src_t,
                       const std::map<size_t,Vi> & srchsh,
                       const std::map<Vi,size_t> & tarhsh,
                       const int & step,
                       const std::map<std::string,int> fv,
                       const std::map<std::string,int> & syms){

    size_t currhsh = tarhsh.at(currpth);
    if (srchsh.find(currhsh)!=srchsh.end())
        return subterm(slv,src_t,srchsh.at(currhsh));
    else {
        CVC::Term node;
        if (fv.find(tar.sym)!=fv.end()) {
            node=slv.mkReal(-(10*step + fv.at(tar.sym)));
        }
        else if (syms.find(tar.sym)!=syms.end()) {
            node=slv.mkReal(syms.at(tar.sym));
         }
        else {
            node=slv.mkReal(strhash(tar.sym));
        }
        Vt args;
        for (int i=0;i!=tar.args.size();i++) {
            Vi newpth;
            for (auto && j : currpth) newpth.push_back(j);
            newpth.push_back(i);
            args.push_back(constructRec(slv,astSort, tar.args.at(i),newpth,src_t,srchsh,tarhsh,
                step, fv,syms));
        }
        return ast(slv, astSort, node, args);
    }
}
