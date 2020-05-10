#ifndef THEORY_H
#define THEORY_H

#include <vector>
#include <string>
#include <set>
#include <map>

typedef std::vector<int> Vi;
typedef std::vector<Vi> Vvi;

// Data Structures
typedef enum { VarNode, AppNode, SortNode} NodeType;

struct Expr {
 const std::string sym;
 const NodeType kind;
 const std::vector<Expr> args;
 Expr(const std::string s,const NodeType k,const std::vector<Expr> a);
 void validate_expr();
};


bool operator==(const Expr& lhs, const Expr& rhs);
bool operator!=(Expr const&x, Expr const&y);

std::ostream & operator << (std::ostream &out, const Expr &e);

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
};

// Simple Constructors of Exprs
Expr Sort(const std::string & sym, const Ve & args=Ve{});
Expr App(const std::string & sym, const Ve & args=Ve{});
Expr Var(const std::string & sym, Expr srt);

std::string join(const Vi & v, const std::string & sep="");

std::map<Vi,size_t> gethash(const Expr e);

std::map<size_t,Vvi> distinct(const std::map<Vi,size_t>  & hashes);

Expr subexpr(const Expr & e, const std::vector<int> & pth);

void addx(std::set<std::string> & syms,const Expr & x,const int & nodet=-1);

std::map<std::string,int> symcode(const Theory & t);

void mergedict(MatchDict acc, const MatchDict & m);

MatchDict patmatch(const Expr &pat, const Expr &x);

Expr sub(const Expr &x, const MatchDict & m);

Expr infer(const std::map<std::string,SortDecl>&  sorts,
             const std::map<std::string,OpDecl> & ops,
             const std::string & sym,
             const Ve & args);

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
int max_arity(const Theory & t);

#endif