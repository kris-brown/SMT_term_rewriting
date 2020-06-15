#include "astextra_basic.hpp"

/*
 * Helper functions for astextra.hpp
 */

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

smt::Term unit(const smt::SmtSolver & slv,
               const smt::Sort & srt,
               const std::string &name) {
    return slv->make_term(smt::Apply_Constructor,slv->get_constructor(srt,name));
}

int arity(const smt::Sort & astSort) {
    return astSort->get_datatype()->get_num_selectors("ast") - 1;
}

smt::Term node(const smt::SmtSolver & slv,
              const smt::Term & x) {
    smt::Term s= slv->get_selector(x->get_sort(),"ast","node");
    return slv->make_term(smt::Apply_Selector, s, x);
}

smt::Term getarg(const smt::SmtSolver & slv,
                const smt::Term & x,
                const int & i) {
    smt::Term s=slv->get_selector(x->get_sort(),"ast","a"+std::to_string(i));
    return slv->make_term(smt::Apply_Selector, s, x);
}


smt::Term ast(const smt::SmtSolver & slv,
              const smt::Sort & astSort,
              const smt::Term & n,
              const Vt & xs
              ) {
    Vt args{slv->get_constructor(astSort, "ast"), n};
    Vt conds;
    smt::Term err=unit(slv, astSort, "Error");
    for (auto && x: xs) {
        args.push_back(x);
        conds.push_back(test(slv,x,"Error"));
    }
    smt::Term condition;
    if (xs.size()==0) condition=slv->make_term(false);
    else if (xs.size()==1) condition=conds.front();
    else condition=slv->make_term(smt::Or, conds);

    smt::Term nt = unit(slv,astSort,"None");
    for (int i=0; i!=arity(astSort)-xs.size(); i++){
        args.push_back(nt);}
    smt::Term ret=slv->make_term(smt::Apply_Constructor,args);
    return slv->make_term(smt::Ite,condition,err,ret);
}

smt::Term replace(const smt::SmtSolver & slv,
                  const int & arg,
                  const smt::Term & x,
                  const smt::Term & y) {
    Vt args;
    smt::Sort astSort=x->get_sort();
    for (int j=0; j!=arity(astSort);j++){
        if (j==arg) args.push_back(y);
        else {
            args.push_back(getarg(slv,x,j));}
    }
    return ast(slv,astSort,node(slv,x),args);
}

smt::Term replP_fun(const smt::SmtSolver & slv,
                    const smt::Term & x,
                    const smt::Term & y,
                    const Vi & p) {
    smt::Term arg=y;
    for (int i=0;i != p.size(); i++) {
        smt::Term subx=x;
        for (int j=p.size()-1;i!=j;j--)
            subx = getarg(slv,subx,p.at(j));
        arg=replace(slv,p.at(i),subx,arg);
    }
    return arg;
}

smt::Term subterm(const smt::SmtSolver & slv,
                   const smt::Term & root,
                   const Vi & pth) {
    smt::Term x = root;
    if (x->get_sort()->to_string()!="AST"){
        throw std::runtime_error("Cannot call subterm on a non-AST");
    }
    //std::cout << "\tin subterm with pth len " << pth.size() << " and got sort " << std::endl;

    smt::Sort astSort = x->get_sort();
    smt::Term sel;
    //std::cout << "\tgot DT " << std::endl;
    for (auto&& i : pth){
        sel = slv->get_selector(astSort, "ast", "a"+std::to_string(i));
        x = slv->make_term(smt::Apply_Selector, sel, x);}
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

smt::Term test(const smt::SmtSolver & slv,
               const smt::Term & x,
               const std::string & s) {
    return slv->make_term(smt::Apply_Tester,slv->get_tester(x->get_sort(),s),x);
}

smt::Term ntest(const smt::SmtSolver & slv,
               const smt::Term & x,
               const std::string & s) {
    return slv->make_term(smt::Not,test(slv,x,s));
}


// Build a term in reference to another term, e.g. y from x
// x=(1+(2+3)) and y=((2+3)+2) then we get y=(x.1 + x.1.0)
smt::Term construct(const smt::SmtSolver & slv,
                    const smt::Sort & astSort,
                    const Theory & t,
                    const Expr & tar,
                    const Expr & src,
                    const smt::Term & src_t,
                    const smt::Term & step) {

    smt::Term step2 = (step != NULL) ? step :
            slv->make_term(0,slv->make_sort(smt::INT));


    std::map<Vi,size_t> srchash, tarh = tar.gethash();
    std::map<std::string,int> fv;
    std::map<size_t,Vi> srch;
    if (src.sym!="?") {
        srchash=src.gethash();
        fv=tar.freevar(src);}
    for (auto && [k,v] : Expr::distinct(srchash)) srch[k]=v.front(); //reverse
    std::map<std::string,int> syms = t.symcode();

    return constructRec(slv,astSort,tar,{},src_t,srch,tarh,step2,fv,syms);
}

smt::Term constructRec(const smt::SmtSolver & slv,
                       const smt::Sort & astSort,
                       const Expr & tar,
                       const Vi & currpth,
                       const smt::Term & src_t,
                       const std::map<size_t,Vi> & srchsh,
                       const std::map<Vi,size_t> & tarhsh,
                       const smt::Term & step,
                       const std::map<std::string,int> fv,
                       const std::map<std::string,int> & syms){

    size_t currhsh = tarhsh.at(currpth);
    smt::Sort Int = slv->make_sort(smt::INT);
    if (srchsh.find(currhsh)!=srchsh.end())
        return subterm(slv,src_t,srchsh.at(currhsh));
    else {
        smt::Term node;
        if (fv.find(tar.sym)!=fv.end()) {
            smt::Term n10 = slv->make_term(-10, Int);
            smt::Term tenstep=slv->make_term(smt::Mult, n10,step);
            smt::Term offset=slv->make_term(fv.at(tar.sym),Int);
            node=slv->make_term(smt::Plus, tenstep, offset);;
        }
        else if (syms.find(tar.sym)!=syms.end()) {
            node=slv->make_term(syms.at(tar.sym),Int);
         }
        else {
            node=slv->make_term(strhash(tar.sym),Int);
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
    std::string sym; Expr::NodeType nt; Ve args;
    long i=std::stol(ast->nodes.at(0)->token);
    for (auto &&[k,v] : t.symcode()) {
        if (v==i){
            sym=k;
            if (t.ops.find(sym)!=t.ops.end()) nt=Expr::AppNode;
            else if(t.sorts.find(sym)!=t.sorts.end()) nt=Expr::SortNode;
            else nt=Expr::VarNode;
        }
    }
    if (sym.empty()){
        nt=Expr::VarNode; sym=strhashinv(i);
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
AST <- '(ast' NUMBER Term* ')'
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
