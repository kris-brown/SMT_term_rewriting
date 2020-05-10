#include <stdexcept>
#include <iostream>
#include <sstream>
#include <src/theory.hpp>

 // Safe constructor
Expr::Expr(const std::string s,const NodeType k,const std::vector<Expr> a) :
    sym(s),kind(k),args(a) {validate_expr();}

void Expr::validate_expr(){
    if (sym.empty())
        throw std::runtime_error("Expr is empty");
    else if (kind==VarNode) {
        if (args.size()!=1)
         throw std::runtime_error("Var does not have 1 arg");
        else if (args.at(0).kind!=SortNode)
         throw std::runtime_error("Var arg is not sort");
    } else if (kind==AppNode) {
        for (int i=0;i!=args.size();i++) {
            if (i>0 && args.at(i).kind==SortNode)
             throw std::runtime_error("Operators cannot operate on types");
        }
    } else if (kind==SortNode) {
        for (auto && a:args) {
            if (a.kind==SortNode)
             throw std::runtime_error("Sorts cannot operate on sorts");
        }

    }
 }



bool operator==(const Expr& lhs, const Expr& rhs){
    return lhs.sym==rhs.sym && lhs.kind==rhs.kind && lhs.args==rhs.args;
}
bool operator!=(Expr const&x, Expr const&y) {
    return !(x==y);
}

std::ostream & operator << (std::ostream &out, const Expr &e)
{
    if (e.kind == VarNode){
        out << e.sym<< ":" << e.args.at(0);
        }
    else {
        NodeType firstchild = e.args.size() ? e.args.at(0).kind : VarNode;
        bool sorted_app= (firstchild==SortNode) && (e.kind==AppNode);
        if (sorted_app && e.args.size()) out << "(";
        out << e.sym;
        if (e.args.size()>(sorted_app ? 1 : 0)) {
            out << "(";
            for (int i=0;i!=e.args.size();i++){
                if (i>0 || !sorted_app)
                    out << e.args.at(i) << ",";
            }
            out << "\b)";
        }
        if (sorted_app){
            if (e.args.size() > (sorted_app ? 1 : 0)) out << ")";
            out << "::" << e.args.at(0);
        }
    }
    return out;
}

// Simple Constructors of Exprs
Expr Sort(const std::string & sym, const Ve & args) {
    return Expr{sym, SortNode, args};
}
Expr App(const std::string & sym, const Ve & args) {
    return Expr{sym, AppNode, args};
}
Expr Var(const std::string & sym, Expr srt) {
    return Expr{sym, VarNode, {srt}};
}

// Join list of ints with an optional separator
std::string join(const Vi & v, const std::string & sep) {
    std::stringstream ss;
    for(size_t i = 0; i < v.size(); ++i) {
        if(i != 0)
            ss << sep;
        ss << v[i];
    }
    std::string s = ss.str();
    return s;
}

// Compute hash of every element in the path
std::map<Vi,size_t> gethash(const Expr e) {
    std::map<Vi,size_t> ret;
    Vi arghashes;
    for (int i=0;i != e.args.size();i++){
        for (auto&& [k,v] :gethash(e.args[i])){
            if (k.empty())
                arghashes.push_back(v);
            Vi newk = k;
            newk.insert(newk.begin() ,i);
            ret[newk]=v;
        }
    }
    size_t curr = std::hash<std::string>()(e.sym+join(arghashes,"|"));
    ret[Vi()] = curr;
    return ret;
}

// Group the subterms by hash
std::map<size_t,Vvi> distinct(const std::map<Vi,size_t>  & hashes){
    std::map<size_t,Vvi> res;
    for (auto&& [k,v] : hashes){
        if (res.find(v)==res.end())
            res[v]={k};
        else
            res[v].push_back(k);
    }
    return res;
}

// Get subexpr from path
Expr subexpr(const Expr & e, const std::vector<int> & pth){
    std::vector<Expr> res{e}; // initial condition
    for (auto i : pth)
        res.emplace_back(res.back().args.at(i));
    return res.back();
}

// Add all strings in the expr, optionally filter by node type
void addx(std::set<std::string> & syms,const Expr & x,const int & nodet){
    if (nodet < 0 || x.kind==nodet) syms.insert(x.sym);
    for (auto && e: x.args) addx(syms,e);
}

std::map<std::string,int> symcode(const Theory & t) {
    std::set<std::string> syms;
    for (auto&& [_,v] : t.sorts)
        {for (auto && e : v.args) addx(syms,e);}
    for (auto&& [_,v] : t.ops) {
        addx(syms,v.sort);
        for (auto && e : v.args) addx(syms,e);
    }
    for (auto&& r : t.rules){
        addx(syms,r.t1); addx(syms,r.t2);}
    std::map<std::string,int> res;
    int i = 1;
    for(auto s : syms) res[s] = i++;
    return res;
}


void mergedict(MatchDict acc, const MatchDict & m){
    for (auto && [k,v] : m) {
        if (acc.find(k)==acc.end()) acc.insert({k,v});
        else if (acc.at(k) != v) acc.insert({"",v});
    }
}

// Match a pattern
MatchDict patmatch(const Expr &pat, const Expr &x){
    MatchDict res;
    if (pat.kind == VarNode){
        res.insert({pat.sym,x}); // add match
        mergedict(res,patmatch(pat.args.at(0),x.args.at(0)));
        return res;}
    else if (x.sym!=pat.sym || x.args.size()!=pat.args.size())
        return {{"",x}}; // error match result
   else{ // Recurse on arguments
    for (int i=0;i!=x.args.size();i++){
        //std::cout << i << " " << x << " " << x.args.size() << " " << pat << " " << pat.args.size() << std::endl;
        mergedict(res,patmatch(pat.args.at(i),x.args.at(i)));}
    return res;}
}

// Substitute any variables in match dictionary into Expr
Expr sub(const Expr &x, const MatchDict & m) {
    if (m.find(x.sym)!=m.end()) return m.at(x.sym);
    else {
        Ve newargs;
        for (auto && a:x.args) newargs.push_back(sub(a,m));
        return {x.sym,x.kind,newargs};
    }
}

//Compute the sort of a function application via pattern matching
Expr infer(const std::map<std::string,SortDecl>&  sorts,
             const std::map<std::string,OpDecl> & ops,
             const std::string & sym,
             const Ve & args) {
    if (ops.find(sym)==ops.end())
        throw std::runtime_error("inferring a symbol not in ops");

    //std::cout << "inferring '" << sym << "' with args:" << std::endl;
    //for (auto && a:args) std::cout << "\t" << a << std::endl;
    Ve op_pat_args = ops.at(sym).args;
    if (op_pat_args.size()!=args.size()){
        std::stringstream buf;
        buf << args.size() << " inferred args" << std::endl;
        buf << op_pat_args.size() << " args in pat " << sym << std::endl;
        for (auto && a:op_pat_args) buf << "\t" << a <<std::endl;
        throw std::runtime_error(buf.str());
    }

    MatchDict match;
    for (int i=0;i!=op_pat_args.size();i++){
        //std::cout << "inferring arg " << i << std::endl;
        Expr x=args.at(i), pat=op_pat_args.at(i);
        MatchDict newmatch = patmatch(pat,x);
        if (newmatch.find("") != newmatch.end()) {
            std::stringstream buf;
            buf << "Pattern match fail for inferring arg " << i <<  std::endl;
            buf << "Arg is " << x << std::endl << "pat is " << pat << std::endl;
            throw std::runtime_error(buf.str());
        }
        mergedict(match, newmatch);
    }
    return sub(ops.at(sym).sort, match);
}

Expr upgrade(const std::map<std::string,SortDecl>&  sorts,
             const std::map<std::string,OpDecl> & ops,
             const Expr & e) {
    //std::cout << "Upgrading Expr " << e << std::endl;
    Ve recargs,newargs;
    for (auto && a : e.args)
        recargs.push_back(upgrade(sorts,ops,a));
    if (e.kind==AppNode)
        newargs.push_back(infer(sorts,ops,e.sym,recargs));
    for (auto && a : recargs) newargs.push_back(a);

    Expr res{e.sym,e.kind,newargs};
    //if (e.kind==AppNode) std::cout << "\tUpgraded Expr " << res << std::endl;
    return res;
}

Expr upgrade(const Theory & t, const Expr & e) {
    return upgrade(t.sorts,t.ops,e);
}

SortDecl upgrade(const std::map<std::string,SortDecl>&  sorts,
                 const std::map<std::string,OpDecl> & ops,
                 const SortDecl & x) {
    //std::cout << "Upgrading SortDecl " << x.sym << std::endl;
    Ve newargs;
    for (auto && a:x.args) newargs.push_back(upgrade(sorts,ops,a));
    return {x.sym, x.pat, newargs, x.desc};
}

OpDecl upgrade(const std::map<std::string,SortDecl>&  sorts,
               const std::map<std::string,OpDecl> & ops,
               const OpDecl & x) {
    //std::cout << "Upgrading OpDecl " << x.sym << std::endl;
    Ve newargs;
    for (auto && a:x.args) newargs.push_back(upgrade(sorts,ops,a));
    return {x.sym, x.pat, upgrade(sorts,ops,x.sort),newargs, x.desc};
}

Rule upgrade(const std::map<std::string,SortDecl>&  sorts,
             const std::map<std::string,OpDecl> & ops,
             const Rule & x) {
    //std::cout << "Upgrading rule " << x.name << std::endl;
    return {x.name,x.desc,upgrade(sorts,ops,x.t1),upgrade(sorts,ops,x.t2)};
}

Theory upgradeT(const Theory & t) {
    std::map<std::string,SortDecl> sorts;
    std::map<std::string,OpDecl> ops;
    std::vector<Rule> rules;
    // In general, we should topologically sort the sorts and operations
    for (auto && [k,v] : t.sorts)
        sorts.insert(std::pair<std::string,SortDecl>(k,upgrade(sorts,ops,v)));
    for (auto && [k,v] : t.ops)
        ops.insert(std::pair<std::string,OpDecl>(k,upgrade(sorts,ops,v)));
    for (auto && r : t.rules) {rules.push_back(upgrade(sorts,ops,r));}
    Theory t2{t.name,sorts,ops,rules};
    return t2;
}

int max_arity(const Theory & t) {
    int m = 0;
    for (auto && [k,v] : t.sorts) {
        if (v.args.size() > m) m=v.args.size();
    }
    for (auto && [k,v] : t.ops) {
        if (v.args.size() > m) m=v.args.size();
    }
    return m;
}
