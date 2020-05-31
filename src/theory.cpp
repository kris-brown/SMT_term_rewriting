#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>

#include "theory.hpp"

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
bool operator==(const SortDecl& lhs, const SortDecl& rhs){
    return lhs.sym==rhs.sym && lhs.pat==rhs.pat && lhs.args==rhs.args && lhs.desc==rhs.desc;
}
bool operator!=(SortDecl const&x, SortDecl const&y) {
    return !(x==y);
}
bool operator==(const OpDecl& lhs, const OpDecl& rhs){
    return lhs.sym==rhs.sym && lhs.pat==rhs.pat && lhs.sort==rhs.sort && lhs.args==rhs.args&& lhs.desc==rhs.desc;
}
bool operator!=(OpDecl const&x, OpDecl const&y) {
    return !(x==y);
}
bool operator==(const Rule& lhs, const Rule& rhs){
    return lhs.name==rhs.name && lhs.t1==rhs.t1 && lhs.t2 ==rhs.t2 && lhs.desc==rhs.desc;
}
bool operator!=(Rule const&x, Rule const&y) {
    return !(x==y);
}
bool operator==(const Theory& lhs, const Theory& rhs){
    return lhs.name==rhs.name && lhs.sorts==rhs.sorts && lhs.ops==rhs.ops && lhs.rules==rhs.rules;
}
bool operator!=(Theory const&x, Theory const&y) {
    return !(x==y);
}

std::string print(const Theory & t, const Expr & e) {
    if (e.kind == VarNode)
        return e.sym + ":" + print(t,e.args.at(0));
    else {
        NodeType firstchild = e.args.size() ? e.args.at(0).kind : VarNode;
        bool sorted_app= (firstchild==SortNode) && (e.kind==AppNode);
        Vs children;

        for (int i=0; i != e.args.size();i++) {
            if ((i!=0) || (!sorted_app)) {
                children.push_back(print(t,e.args.at(i)));
            }
        }

        std::string pat= ((e.kind==SortNode) ? t.sorts.at(e.sym).pat : t.ops.at(e.sym).pat);
        Vs syms=split(pat,"{}");
        std::string ret;
        for (int i=0;i!=children.size();i++)
            ret+=syms.at(i) + children.at(i);
        //std::cout << e << " is printed out as " << ret+syms.back() << std::endl;
        return ret+syms.back();
    }
}

std::string print(const Theory & t, const SortDecl & x){
    Vs args;
    for (auto && a:x.args) args.push_back(print(t,uninfer(a)));
    return "Sort: "+x.sym+" "+x.pat+"\n\t"+join(args,"\n\t");
}
std::string print(const Theory & t, const OpDecl & x){
    Vs args;
    for (auto && a:x.args) args.push_back(print(t,uninfer(a)));
    return "Op: "+x.sym+" "+x.pat+"\n\t"+join(args,"\n\t");

}
std::string print(const Theory & t, const Rule & x, const int & dir){
    std::string p1="\t",p2="\t";
    switch (dir) {
        case 1:
            p1 += "⟵\t";
            p2 += "⟶\t";
            break;
        case 0:
            p2 += "⟵\t";
            p1 += "⟶\t";
            break;
    }
    return "Rule: "+x.name+"\n"+p1+print(t,uninfer(x.t1))
            +"\n"+p2+print(t,uninfer(x.t2));
}

std::string print(const Theory & t){
    Vs lines;
    for (auto &&[v,s]:t.sorts) lines.push_back(print(t,s));
    for (auto &&[v,s]:t.ops) lines.push_back(print(t,s));
    for (auto &&r:t.rules) lines.push_back(print(t,r));

    return t.name+"\n\n"+join(lines,"\n");
}

std::ostream & operator << (std::ostream &out, const Expr &e) {
    if (e.kind == VarNode)
        out << e.sym<< ":" << e.args.at(0);
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

 // Safe constructor
 std::map<std::string,SortDecl> Theory::make_sdict(std::vector<SortDecl> s) {
     std::map<std::string,SortDecl> sdict;
    for (auto && sd : s) sdict.insert({sd.sym,sd});
    return sdict;
}
 std::map<std::string,OpDecl> Theory::make_odict(std::vector<OpDecl> o) {
     std::map<std::string,OpDecl> odict;
    for (auto && od : o) odict.insert({od.sym,od});
    return odict;
}

Theory::Theory() : name("Default"), sorts({}), ops({}), rules ({}) {}

Theory::Theory(const std::string n,
           const std::vector<SortDecl> s,
           const std::vector<OpDecl> o,
           const std::vector<Rule> r) :
    name(n),
    sorts(make_sdict(s)),
    ops(make_odict(o)),
    rules(r)
    {validate_theory();}

Theory::Theory(const std::string n,
           const std::map<std::string,SortDecl> s,
           const  std::map<std::string,OpDecl> o,
           const std::vector<Rule> r) :
    name(n), sorts(s), ops(o), rules(r) {validate_theory();}

Theory parseTheory(const std::string pth) {
        peg::parser parser(R"(
Items <- WORD Item*
Item <- SortDecl / OpDecl / Rule
SortDecl <- 'Sort' WORD PHRASE PHRASE '[' Term* ']'
OpDecl <- 'Op' WORD PHRASE PHRASE Term '[' Term* ']'
Rule <- 'Rule' WORD PHRASE Term Term
Term <- Var / WORD '(' Term* ')' / WORD
Var <- WORD ':' Term
WORD <- < [a-zA-Z_] [a-zA-Z0-9_]* >
PHRASE <- < '"' (!'"' .)* '"' >
%whitespace  <-  [ \t\r\n,]*
)");
    assert((bool)parser == true);
    parser.enable_ast();
    parser.enable_packrat_parsing();
    std::ifstream infile(pth);
    if (infile.fail()){
        infile.close();
        throw std::runtime_error("Bad path to theory file: "+pth);
    }
    std::string content((std::istreambuf_iterator<char>(infile) ),
                        (std::istreambuf_iterator<char>()    ) );
    std::shared_ptr<peg::Ast> ast;
    if (parser.parse(content.c_str(), ast)) {
        ast = peg::AstOptimizer(true).optimize(ast);

        std::string name=ast->nodes.at(0)->token;
        KD kinddict;
        for (int i=1;i<ast->nodes.size();i++) {
            std::shared_ptr<peg::Ast> item=ast->nodes.at(i);
            if (item->name=="SortDecl") kinddict.insert({item->nodes.at(0)->token,SortNode});
            if(item->name == "OpDecl") kinddict.insert({item->nodes.at(0)->token,AppNode});
        }
        std::vector<SortDecl> sorts;
        std::vector<OpDecl> ops;
        std::vector<Rule> rules;
        for (int i=1;i<ast->nodes.size();i++) {
            std::shared_ptr<peg::Ast> item=ast->nodes.at(i);
            if (item->name=="SortDecl") sorts.push_back(parseSort(item,kinddict));
            else if (item -> name == "OpDecl") ops.push_back(parseOp(item,kinddict));
            else if (item -> name == "Rule") rules.push_back(parseRule(item,kinddict));
            else {throw std::runtime_error("Bad theory file: "+pth+ " has top-level "+(item->name));}
        }
        return {name,sorts,ops,rules};
    }
    else {
        throw std::runtime_error("syntax error in theory file:"+pth);
    }
}

Expr parseExpr(std::shared_ptr<peg::Ast> ast, KD kd) {
    std::string name;
    Vx args;
    if (ast->nodes.empty()) name = ast->token;
    else {
        for (int i=0;i!=ast->nodes.size();i++) {
            if (i==0) name = ast->nodes.at(i)->token;
            else  args.push_back(parseExpr(ast->nodes.at(i), kd));
        }
    }

    NodeType nt;
    if (kd.find(name)==kd.end()) nt = VarNode;
    else if (kd.at(name) == AppNode) nt = AppNode;
    else nt = SortNode;
    Expr out{name,nt,args};
    return out;
}

SortDecl parseSort(std::shared_ptr<peg::Ast> ast, KD kd) {
    Ve args;
    for (int i=3;i!=ast->nodes.size();i++)
        args.push_back(parseExpr(ast->nodes.at(i), kd));

    return {ast->nodes.at(0)->token,
            trim(ast->nodes.at(1)->token),
            args, trim(ast->nodes.at(2)->token)};
}

OpDecl parseOp(std::shared_ptr<peg::Ast> ast, KD kd) {
    Ve args;
    for (int i=4;i!=ast->nodes.size();i++)
        args.push_back(parseExpr(ast->nodes.at(i), kd));
    return {ast->nodes.at(0)->token,
            trim(ast->nodes.at(1)->token),
            parseExpr(ast->nodes.at(3),kd),
            args, trim(ast->nodes.at(2)->token)};
}
Rule parseRule(std::shared_ptr<peg::Ast> ast, KD kd) {
    return {ast->nodes.at(0)->token,
            trim(ast->nodes.at(1)->token),
            parseExpr(ast->nodes.at(2),kd),
            parseExpr(ast->nodes.at(3),kd)};
}

void Theory::validate_theory(){
    // TBD
}

void Theory::validate_sorted_theory(){
    // TBD
}

// Simple Constructors of Exprs
Expr Srt(const std::string & sym, const Ve & args) {
    return Expr{sym, SortNode, args};
}
Expr App(const std::string & sym, const Ve & args) {
    return Expr{sym, AppNode, args};
}
Expr Var(const std::string & sym, Expr srt) {
    return Expr{sym, VarNode, {srt}};
}

// Join list of ints with an optional separator
template<typename T>
std::string join(const std::vector<T> & v, const std::string & sep) {
    std::stringstream ss;
    for(size_t i = 0; i < v.size(); ++i) {
        if(i != 0)
            ss << sep;
        ss << v[i];
    }
    std::string s = ss.str();
    return s;
}
std::string trim(const std::string & x) {
    return x.substr(1,x.length()-2);
}

// Compute hash of every element in the path
std::map<Vi,size_t> gethash(const Expr & e) {
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
    for (auto && e: x.args) addx(syms,e, nodet);
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
std::string symcodestr(const Theory & t) {
    std::map<std::string,int> code=symcode(t);
    std::stringstream buf;
    for (auto &&[k,v] : code) buf << " | " << k << " " << v ;
    return buf.str();
}

void mergedict(MatchDict & acc, const MatchDict & m){
    for (auto && [k,v] : m) {
        //std::cout << "Adding " << k << " -> " << v << std::endl;
        if (acc.find(k)==acc.end()) acc.insert({k,v});
        else if (acc.at(k) != v) {
            acc.insert({"",v});}
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
        throw std::runtime_error("inferring a symbol ("+sym+")not in ops");
    for (auto && a:args) {
        if (a.kind==SortNode)
            throw std::runtime_error("inferring a term that has a Sort as a direct argument: is this term already inferred?");
    }

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
        Expr x=args.at(i), pat=op_pat_args.at(i);
        MatchDict newmatch = patmatch(pat,x);
        if (newmatch.find("") != newmatch.end()) {
            std::stringstream buf;
            buf << sym << " pattern match fail for inferring arg " << i <<  std::endl;
            buf << "Arg is " << x << std::endl << "pat is " << pat << std::endl;
            throw std::runtime_error(buf.str());
        }
        mergedict(match, newmatch);

        if (match.find("") != match.end()) {
            std::stringstream buf;
            buf << sym << " pattern match fail given conflicting args after adding arg " << i << std::endl;
            for (auto && a:args) buf << "\n\t" << a << std::endl;
            throw std::runtime_error(buf.str());
        }

    }

    return sub(ops.at(sym).sort, match);
}

Expr upgrade(const std::map<std::string,SortDecl>&  sorts,
             const std::map<std::string,OpDecl> & ops,
             const Expr & e) {
    Ve recargs,newargs;
    for (auto && a : e.args)
        recargs.push_back(upgrade(sorts,ops,a));
    if (e.kind==AppNode)
        newargs.push_back(infer(sorts,ops,e.sym,recargs));
    for (auto && a : recargs) newargs.push_back(a);

    Expr res{e.sym,e.kind,newargs};
    return res;
}

Expr upgrade(const Theory & t, const Expr & e) {
    return upgrade(t.sorts,t.ops,e);
}

SortDecl upgrade(const std::map<std::string,SortDecl>&  sorts,
                 const std::map<std::string,OpDecl> & ops,
                 const SortDecl & x) {
    Ve newargs;
    for (auto && a:x.args) newargs.push_back(upgrade(sorts,ops,a));
    return {x.sym, x.pat, newargs, x.desc};
}

OpDecl upgrade(const std::map<std::string,SortDecl>&  sorts,
               const std::map<std::string,OpDecl> & ops,
               const OpDecl & x) {
    Ve newargs;
    for (auto && a:x.args) newargs.push_back(upgrade(sorts,ops,a));
    return {x.sym, x.pat, upgrade(sorts,ops,x.sort),newargs, x.desc};
}

Rule upgrade(const std::map<std::string,SortDecl>&  sorts,
             const std::map<std::string,OpDecl> & ops,
             const Rule & x) {
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
    t2.validate_sorted_theory();
    return t2;
}

int max_arity(const Theory & t) {
    int m = 1;
    for (auto && [k,v] : t.sorts) {
        if (v.args.size() > m) m=v.args.size();
    }
    for (auto && [k,v] : t.ops) {
        if (v.args.size() > m) m=v.args.size();
    }
    return m;
}

Vs split(const std::string& s, const std::string & delim)
{
    Vs res;
    auto start = 0U;
    auto end = s.find(delim);
    while (end != std::string::npos)
    {
      res.push_back(s.substr(start, end - start));
        start = end + delim.length();
        end = s.find(delim, start);
    }
    res.push_back(s.substr(start, end));
    return res;
}

bool isSpace(unsigned char c) {
	return (c == ' ' || c == '\n' || c == '\r' ||
		   c == '\t' || c == '\v' || c == '\f');
}

void del_whitespace(std::string s){
    s.erase(std::remove_if(s.begin(), s.end(), isSpace), s.end());
}

std::string mkParser(const std::string & pat) {
    std::stringstream ss;
    Vs raw=split(pat,"{}");
    for (int i=0;i!=raw.size()-1;i++){
        std::string s=raw.at(i);
        std::regex r("\\s+");
        s = std::regex_replace(s, r, "");
        ss << "'" << s << "' Term ";}
    ss << "'" << raw.back() << "'\n";
    return ss.str();
}
std::string mkParser(const Theory & t) {
    std::stringstream ss;
    ss << "Term <- Var";
    for (auto && [k,v] : t.ops) ss << " / " << k;
    ss << "\nSort <- ";
    int i=0;
    for (auto && p : t.sorts) {
        if (i!=0) ss << " / ";
        ss << p.first;
        i++;
    }
    ss << '\n';
    for (auto && [k,v] : t.sorts) ss << k << " <-  " << mkParser(v.pat);
    ss << "\n";
    for (auto && [k,v] : t.ops) ss << k << " <-  " << mkParser(v.pat);
    ss << "Var <- WORD ':' Sort\nWORD <- < [a-zA-Z] [a-zA-Z0-9]* >\n%whitespace  <-  [ \\t\\r\\n]*";
    return ss.str();
}

Vx parse_exprs(const Theory & t, const std::string & pth) {
    std::ifstream infile(pth);
    if (infile.fail()){
        infile.close();
        throw std::runtime_error("Bad path to file with expressions: "+pth);
    }
    Vx res;
    std::string line;
    while (std::getline(infile, line))
        res.push_back(parse_expr(t,line));
    infile.close();
    return res;
}

Expr parse_expr(const Theory & t, const std::string & expr) {

    peg::parser parser(mkParser(t).c_str());
    parser.enable_ast();
    assert((bool)parser == true);

    std::shared_ptr<peg::Ast> ast;
    if (parser.parse(expr.c_str(), ast)) {
        return ast_to_expr(t, ast);
    }
    else {
        throw std::runtime_error("syntax error: " + expr);
    }
}

Expr ast_to_expr(const Theory & t, const std::shared_ptr<peg::Ast> & ast) {
    Expr res{"x",AppNode,{}};
    std::string kind=ast->name;

    if ((kind != "Term" && kind != "Sort") || ast->nodes.size() != 1)
        throw std::runtime_error("Bad AST. Name = "+ast->name);

    std::shared_ptr<peg::Ast> child=(ast->nodes).at(0);
    std::string name=child->name;

    if (name=="Var") {
        Vx srt{ast_to_expr(t,child->nodes.at(1))};
        return Expr(child->nodes.at(0)->token, VarNode, srt);
    }

    Vx children;
    for (auto node : child->nodes) {
      children.push_back(ast_to_expr(t,node));
    }

    if (kind=="Term") {
        return Expr(name, AppNode, children);
    } else if (kind=="Sort") {
        return Expr(name, SortNode, children);
    }
     else {
            throw std::runtime_error("Unknown symbol");
     }
}

std::map<std::string,int> freevar(const Expr & x, const Expr & y) {
    std::set<std::string> symx,symy;
    addx(symx,x,VarNode); addx(symy,y,VarNode);

    std::map<std::string,int> res;
    int i = 1;
    for (auto && e : symx){
    if (symy.find(e)==symy.end())
        res[e]=i++;}
    return res;
}


Expr uninfer(const Expr & x) {
    Vx newargs;
    for (auto && a:x.args) {
        if ((x.kind != AppNode) || (a.kind!=SortNode))
            newargs.push_back(uninfer(a));
    }
    return {x.sym,x.kind,newargs};
}
