#ifndef THEORY_H
#define THEORY_H

#include <vector>
#include <string>
#include <set>
#include <map>
#include "../external/peglib.h"

typedef std::vector<int> Vi;
typedef std::vector<Vi> Vvi;
typedef std::vector<std::string> Vs;

// Data Structures
typedef enum { VarNode, AppNode, SortNode} NodeType;

typedef std::map<std::string,NodeType> KD;

struct Expr {
 const std::string sym;
 const NodeType kind;
 const std::vector<Expr> args;
 Expr(const std::string s,const NodeType k,const std::vector<Expr> a);
 void validate_expr();
};
typedef std::vector<Expr> Vx;

typedef std::vector<Expr> Ve;
typedef std::map<std::string,Expr> MatchDict;

struct SortDecl {
    const std::string sym;
    const std::string pat;
    const Ve args;
    const std::string desc;
};

struct OpDecl {
    const std::string sym;
    const std::string pat;
    const Expr sort;
    const Ve args;
    const std::string desc;
};

struct Rule {
    const std::string name;
    const std::string desc;
    const Expr t1;
    const Expr t2;
};

struct Theory {
    const std::string name;
    const std::map<std::string,SortDecl> sorts;
    const std::map<std::string,OpDecl> ops;
    const std::vector<Rule> rules;
    Theory();
    Theory(const std::string n,
           const std::vector<SortDecl> s,
           const std::vector<OpDecl> o,
           const std::vector<Rule> r);
    Theory(const std::string n,
           const std::map<std::string,SortDecl> s,
           const std::map<std::string,OpDecl> o,
           const std::vector<Rule> r);

    void validate_sorted_theory();
    private :
    void validate_theory();
    std::map<std::string,SortDecl> make_sdict(std::vector<SortDecl> s);
    std::map<std::string,OpDecl> make_odict(std::vector<OpDecl> o);
};

// Equality
bool operator==(const Expr& lhs, const Expr& rhs);
bool operator!=(Expr const&x, Expr const&y);
bool operator==(const SortDecl& lhs, const SortDecl& rhs);
bool operator!=(SortDecl const&x, SortDecl const&y);
bool operator==(const OpDecl& lhs, const OpDecl& rhs);
bool operator!=(OpDecl const&x, OpDecl const&y);
bool operator==(const Rule& lhs, const Rule& rhs);
bool operator!=(Rule const&x, Rule const&y);
bool operator==(const Theory& lhs, const Theory& rhs);
bool operator!=(Theory const&x, Theory const&y);

// Parsing theories from files
Theory parseTheory(const std::string pth);
SortDecl parseSort(std::shared_ptr<peg::Ast> ast, KD kd);
OpDecl parseOp(std::shared_ptr<peg::Ast> ast, KD kd);
Rule parseRule(std::shared_ptr<peg::Ast> ast, KD kd);
Expr parseExpr(std::shared_ptr<peg::Ast> ast, KD kd);

// Simple Constructors of Exprs
Expr Sort(const std::string & sym, const Ve & args=Ve{});
Expr App(const std::string & sym, const Ve & args=Ve{});
Expr Var(const std::string & sym, Expr srt);

// String manipulation
template<typename T>
std::string join(const std::vector<T> & v, const std::string & sep="");
Vs split(const std::string& s, const std::string & delim);
std::string trim(const std::string & x);
bool isSpace(unsigned char c);
void del_whitespace(std::string s);

// Rendering
std::ostream & operator << (std::ostream &out, const Expr &e);
std::string print(const Theory & t, const Expr & e); // Inverse to parse_expr
std::string print(const Theory & t);
std::string print(const Theory & t, const SortDecl & x);
std::string print(const Theory & t, const OpDecl & x);
std::string print(const Theory & t, const Rule & x);

// Paritioning
std::map<Vi,size_t> gethash(const Expr & e);
std::map<size_t,Vvi> distinct(const std::map<Vi,size_t>  & hashes);

// Basic type inference
Expr subexpr(const Expr & e, const std::vector<int> & pth);
void addx(std::set<std::string> & syms,const Expr & x,const int & nodet=-1);
std::map<std::string,int> symcode(const Theory & t);
std::string symcodestr(const Theory & t);
void mergedict(MatchDict & acc, const MatchDict & m);
MatchDict patmatch(const Expr &pat, const Expr &x);
Expr sub(const Expr &x, const MatchDict & m);
Expr infer(const std::map<std::string,SortDecl>&  sorts,
             const std::map<std::string,OpDecl> & ops,
             const std::string & sym,
             const Ve & args);
Expr uninfer(const Expr & x); // undo infer

// Upgrading unsorted terms to sorted terms
Expr upgrade(const std::map<std::string,SortDecl>&  sorts,
             const std::map<std::string,OpDecl> & ops,
             const Expr & e);
Expr upgrade(const Theory & t, const Expr & e);
SortDecl upgrade(const std::map<std::string,SortDecl>&  sorts,
                 const std::map<std::string,OpDecl> & ops,
                 const SortDecl & x);
OpDecl upgrade(const std::map<std::string,SortDecl>&  sorts,
               const std::map<std::string,OpDecl> & ops,
               const OpDecl & x);
Rule upgrade(const std::map<std::string,SortDecl>&  sorts,
             const std::map<std::string,OpDecl> & ops,
             const Rule & x);
Theory upgradeT(const Theory & t);

// Basic info
int max_arity(const Theory & t);

// Parsing
std::string mkParser(const std::string & pat);
std::string mkParser(const Theory & t);
Vx parse_exprs(const Theory & t, const std::string & pth);
Expr parse_expr(const Theory & t, const std::string & expr); // inverse to print
Expr ast_to_expr(const Theory & t, const std::shared_ptr<peg::Ast> & ast);

// Misc
std::map<std::string,int> freevar(const Expr & x, const Expr & y);

#endif