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
    Vt conds;
    CVC::Term err=unit(slv,astSort,"Error");
    for (auto && x: xs) {
        args.push_back(x);
        conds.push_back(test(slv,x,"Error"));
    }
    CVC::Term condition;
    if (xs.size()==0) condition=slv.mkFalse();
    else if (xs.size()==1) condition=conds.front();
    else condition=slv.mkTerm(CVC::OR, conds);

    CVC::Term nt = unit(slv,astSort,"None");
    for (int i=0; i!=arity(astSort)-xs.size(); i++){
        args.push_back(nt);}
    CVC::Term ret=slv.mkTerm(CVC::APPLY_CONSTRUCTOR,args);
    return slv.mkTerm(CVC::ITE,condition,err,ret);
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
    if (x.getSort().toString()!="AST"){
        throw std::runtime_error("Cannot call subterm on a non-AST");
    }
    //std::cout << "\tin subterm with pth len " << pth.size() << " and got sort " << std::endl;

    CVC::DatatypeConstructor s = x.getSort().getDatatype().getConstructor("ast");
    CVC::Term sel;
    //std::cout << "\tgot DT " << std::endl;
    for (auto&& i : pth){
        sel = s.getSelectorTerm("a"+std::to_string(i));
        x = slv.mkTerm(CVC::APPLY_SELECTOR, sel, x);}
    return x;
}


int64_t strhash(const std::string & s) {
    std::stringstream ss;
    for (auto && i:s) ss << std::to_string(i-30);
    return std::stol(ss.str());
}
std::string strhashinv(const int64_t & i) {
    if (i > 0) {
    std::stringstream ss;
    std::string s=std::to_string(i);
    for (int j=0;j!=s.size()/2;j++) {
        ss << (char) (std::stoi(s.substr(2*j,2))+30);}
    return ss.str();
    } else if (i < 0) {
        return "FREE"+std::to_string(std::abs(i));
    }
    else {
         throw std::runtime_error("Cannot have variable code 0");
    }
}

CVC::Term test(const CVC::Solver & slv,
               const CVC::Term & x,
               const std::string & s) {
    return slv.mkTerm(CVC::APPLY_TESTER,x.getSort().getDatatype().getConstructor(s).getTesterTerm(),x);
}

CVC::Term ntest(const CVC::Solver & slv,
               const CVC::Term & x,
               const std::string & s) {
    return slv.mkTerm(CVC::NOT,test(slv,x,s));
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

Expr parseCVCast(const Theory & t, std::shared_ptr<peg::Ast> ast) {
    std::string sym; NodeType nt; Vx args;
    long i=std::stol(ast->nodes.at(0)->token);
    for (auto &&[k,v] : symcode(t)) {
        if (v==i){
            sym=k;
            if (t.ops.find(sym)!=t.ops.end()) nt=AppNode;
            else if(t.sorts.find(sym)!=t.sorts.end()) nt=SortNode;
            else nt=VarNode;
        }
    }
    if (sym.empty()){
        nt=VarNode; sym=strhashinv(i);
    }
    for (int i=0;i != ast->nodes.size(); i++) {
        std::shared_ptr<peg::Ast> a=ast->nodes.at(i);
        if (a->nodes.size()) {
            args.push_back(parseCVCast(t,a->nodes.at(0)));
        }
    }
    return {sym,nt,args};
}
Expr parseCVC(const Theory & t,
              const std::string s)  {

    peg::parser parser(R"(
AST <- 'ast(' NUMBER Term* ')'
Term <- AST / 'None'
NUMBER <- < '-'? [0-9]+ >
%whitespace  <-  [ \t\r\n,]*
)");

    assert((bool)parser == true);

    parser.enable_ast();
    parser.enable_packrat_parsing();
    std::shared_ptr<peg::Ast> ast;

    if (parser.parse(s.c_str(), ast)) {
        return parseCVCast(t,ast);

    } else {
         throw std::runtime_error("syntax error in CVC term:"+s);

    }

}
