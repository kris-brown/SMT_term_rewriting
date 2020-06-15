#ifndef THEORY_H
#define THEORY_H

/*
 * Data structures and functions associated with GATs
 */

#include <vector>
#include <string>
#include <set>
#include <map>

#include "../external/peglib.h"

// Forward declarations
struct Expr;
struct OpDecl;
struct SortDecl;

// Abbreviations
typedef std::vector<int> Vi;
typedef std::vector<Vi> Vvi;
typedef std::vector<std::string> Vs;
typedef std::vector<Expr> Ve;
typedef std::map<std::string,Expr> MatchDict;

// Data Structures

/**
 * Term in a theory
 */

struct Expr {

 public:
    typedef enum { VarNode, AppNode, SortNode} NodeType;
    const std::string sym;
    const NodeType kind;
    const std::vector<Expr> args;
    Expr(const std::string s,const NodeType k,const std::vector<Expr> a);

    Expr upgrade(const std::map<std::string,SortDecl>&  sorts,
                   const std::map<std::string,OpDecl>& ops) const;

    // Compute hash of every element in the path
    std::map<Vi,size_t> gethash() const;

    // Substitute any variables in match dictionary into Expr
    Expr sub(const MatchDict & m) const;

    Expr uninfer() const; // undo infer

    // Access subterm, specified by path (the i'th arg of the j'th arg of the ...)
    Expr subexpr(const Vi & pth) const;


    static Expr infer(const std::map<std::string,SortDecl>&  sorts,
                      const std::map<std::string,OpDecl> & ops,
                      const std::string & sym,
                      const Ve & args);

    bool operator==(const Expr & that) const;
    bool operator!=( Expr const & that) const ;
    friend std::ostream & operator << (std::ostream &out, const Expr &e);

    static std::map<size_t,Vvi> distinct(const std::map<Vi,size_t>  & hashes);

    // Basic type inference

    static void mergedict(MatchDict & acc, const MatchDict & m);
    MatchDict patmatch(const Expr &x) const;
    std::map<std::string,int> freevar(const Expr & y) const;

    void addx(std::set<std::string> & syms,const int & nodet=-1) const;

 private:
    void validate_expr();



};


/**
 * Specification of a sort within a theory
 */

struct SortDecl {
    public:
        const std::string sym;
        const std::string pat;
        const Ve args;
        const std::string desc;

        bool operator==(const SortDecl &that) const;
        bool operator!=(const SortDecl &that) const;

        SortDecl upgrade(const std::map<std::string,SortDecl>& sorts,
                         const std::map<std::string,OpDecl> & ops) const;
};

/**
 * Specification of an operator within a theory
 */

struct OpDecl {
    public:
        const std::string sym;
        const std::string pat;
        const Expr sort;
        const Ve args;
        const std::string desc;

        bool operator==(const OpDecl& that) const;
        bool operator!=(const OpDecl& that) const;

        OpDecl upgrade(const std::map<std::string,SortDecl>&  sorts,
                    const std::map<std::string,OpDecl> & ops) const;

};

/**
 * Specification of an equivalence rule within a theory
 */

struct Rule {
    public:
        const std::string name;
        const std::string desc;
        const Expr t1;
        const Expr t2;

        Rule upgrade(const std::map<std::string,SortDecl>&  sorts,
                    const std::map<std::string,OpDecl> & ops) const;

        bool operator==(const Rule & that) const;
        bool operator!=(const Rule & that) const;

};

/**
 * Specification of a generalized algebraic theory
 */
typedef std::map<std::string,Expr::NodeType> KD;

struct Theory {
    public:
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
        bool operator==(const Theory& that) const;
        bool operator!=(const Theory& that) const;

        std::string print(const Expr & e) const; // Inverse to parse_expr
        std::string print();
        std::string print(const SortDecl & x) const;
        std::string print(const OpDecl & x) const;
        std::string print(const Rule & x, const int & dir=-1) const;

        int max_arity() const;

        Expr upgrade(const Expr & e) const;
        Theory upgrade() const;


        Ve parse_exprs(const std::string & pth) const;
        Expr parse_expr(const std::string & expr) const; // inverse to print
        static Theory parseTheory(const std::string pth);

        std::map<std::string,int> symcode() const;
    private:
        void validate_theory();
        std::map<std::string,SortDecl> make_sdict(std::vector<SortDecl> s);
        std::map<std::string,OpDecl> make_odict(std::vector<OpDecl> o);

        static std::string strParser(const std::string & pat);
        std::string mkParser() const;
        Expr ast_to_expr(const std::shared_ptr<peg::Ast> & ast) const;

        static SortDecl parseSort(std::shared_ptr<peg::Ast> ast, KD kd);
        static OpDecl parseOp(std::shared_ptr<peg::Ast> ast, KD kd);
        static Rule parseRule(std::shared_ptr<peg::Ast> ast, KD kd);
        static Expr parseExpr(std::shared_ptr<peg::Ast> ast, KD kd);

};

// Simple Constructors of Exprs
Expr Srt(const std::string & sym, const Ve & args=Ve{});
Expr App(const std::string & sym, const Ve & args=Ve{});
Expr Var(const std::string & sym, Expr srt);

// String manipulation
template<typename T>
std::string join(const std::vector<T> & v, const std::string & sep="");
Vs split(const std::string& s, const std::string & delim);
std::string trim(const std::string & x);
bool isSpace(unsigned char c);
void del_whitespace(std::string s);

#endif