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
typedef std::map<std::string, Expr> MatchDict;
typedef std::map<std::string, SortDecl> SortDeclDict;
typedef std::map<std::string, OpDecl> OpDeclDict;

/**
 * Term in a theory
 */
struct Expr
{

public:
    // Nodes in term graph can be either variables, sorts, or the application of some operator
    typedef enum
    {
        VarNode,
        AppNode,
        SortNode
    } NodeType;

    // Symbol of top-level node
    const std::string sym;

    // Nodetype of top-level node
    const NodeType kind;

    // Arguments of an AppNode, i.e. term level operator (like + 1 1)
    // or for a SortNode, e.g. Hom(A:Ob, B:Ob)
    const std::vector<Expr> args;

    // Safe constructor
    Expr(const std::string s, const NodeType k, const std::vector<Expr> a);

    /**
     * Elaborate type information
     */
    Expr upgrade(const SortDeclDict &sorts,
                 const OpDeclDict &ops) const;

    /**
     * Compute hash of every element indexed by its subpath
     */
    std::map<Vi, size_t> gethash() const;

    /**
     * Substitute any variables in match dictionary into Expr
     * @param matchdict Mapping of variables to substitute
     */
    Expr sub(const MatchDict &matchdict) const;

    /**
     * Remove elaborated type info
     */
    Expr uninfer() const;

    /**
     * Access a subterm, specified by path (take the i'th arg, then the j'th arg of that, then the ...)
     * @param pth List of argument indices to take, in order
     * @returns A subterm (index error if not possible)
     */
    Expr subexpr(const Vi &pth) const;

    bool operator==(const Expr &that) const;

    bool operator!=(Expr const &that) const;

    friend std::ostream &operator<<(std::ostream &out, const Expr &e);

    /**
     * Group all subterms by hash
     * @param hashes Mapping of subpaths to their hashes, assumed to be the result of calling gethash()
     * @returns a mapping from hashes to a list of subpaths that share the same value
     */
    static std::map<size_t, Vvi> distinct(const std::map<Vi, size_t> &hashes);

    /**
     * Compute the sort of a function application via pattern matching
     * @param sorts SortDecls of a theory
     * @param ops OpDecls of a theory
     * @param sym operator symbol being applied
     * @param args args to which operator is applied
     * @returns a SortNode expr, according to the theory being used
     */
    static Expr infer(const SortDeclDict &sorts,
                      const OpDeclDict &ops,
                      const std::string &sym,
                      const Ve &args);

    /**
     * Combine two dictionaries
     */
    static void mergedict(MatchDict &acc, const MatchDict &m);

    /**
     * Match a pattern expression (*this*) with an argument expr
     * If failure, return dict with "" key.
     * @param x Expression to which pattern (this) is matched
     * @returns a mapping from the variables in *this* to substerms in x, if possible
     */
    MatchDict patmatch(const Expr &x) const;

    /**
     * Variables that are not in the argument expr but found in *this*
     * (each given a unique integer starting at 1)
     * @param y Background term with variables which should be considered as bound
     * @returns Numbered variables that do not appear in y
     */
    std::map<std::string, int> freevar(const Expr &y) const;

    /**
     * Collect all symbols in the expr, (impure function modifies its arg)
     * @param syms Set of symbols to be added to
     * @param nodetype Optional filter to only add symbols from a particular NodeType (-1 means no filter)
     */
    void addx(std::set<std::string> &syms, const int &nodetype = -1) const;

private:
    void validate_expr();
};

typedef std::map<std::string, Expr::NodeType> KindDict;

/**
 * Specification of a sort within a theory
 */
struct SortDecl
{
public:
    // Unique string of the sort, e.g. Int/Ob/Hom
    const std::string sym;
    // How terms with this sort should be printed and parsed, e.g. "({}⇒{})"
    const std::string pat;
    // Canonical arguments, e.g. [A:Ob,B:Ob] for defining 'Hom' as Hom(A,B)
    const Ve args;
    // Description
    const std::string desc;

    bool operator==(const SortDecl &that) const;
    bool operator!=(const SortDecl &that) const;

    /**
     * Elaborate type information
     */
    SortDecl upgrade(const SortDeclDict &sorts,
                     const OpDeclDict &ops) const;
};

/**
 * Specification of an operator within a theory
 */
struct OpDecl
{
public:
    // Unique string of the sort, e.g. Plus
    const std::string sym;
    // How terms with this sort should be printed and parsed, e.g. "({}+{})"
    const std::string pat;
    // The result sort of applying the function to the canonical arguments,
    // e.g. for composition, Hom(A,C) given arguments [Hom(A,B),Hom(B,C)]
    const Expr sort;
    // Canonical arguments
    const Ve args;
    // Description
    const std::string desc;

    bool operator==(const OpDecl &that) const;
    bool operator!=(const OpDecl &that) const;

    /**
     * Elaborate type information
     */
    OpDecl upgrade(const SortDeclDict &sorts,
                   const OpDeclDict &ops) const;
};

/**
 * Specification of an equivalence rule within a theory between two term patterns
 */
struct Rule
{
public:
    const std::string name;
    const std::string desc;
    const Expr t1;
    const Expr t2;

    /**
     *  Elaborate type information
     */
    Rule upgrade(const SortDeclDict &sorts,
                 const OpDeclDict &ops) const;

    bool operator==(const Rule &that) const;
    bool operator!=(const Rule &that) const;
};

/**
 * Specification of a generalized algebraic theory
 */
struct Theory
{
public:
    const std::string name;
    const SortDeclDict sorts;
    const OpDeclDict ops;
    const std::vector<Rule> rules;

    /**
     * @returns Default theory
     */
    Theory();

    /**
     * Create dictionaries indexed by Sort/Op symbols
     */
    Theory(const std::string n,
           const std::vector<SortDecl> s,
           const std::vector<OpDecl> o,
           const std::vector<Rule> r);

    /**
     * Safe constructor
     */
    Theory(const std::string n,
           const SortDeclDict s,
           const OpDeclDict o,
           const std::vector<Rule> r);

    // TODO
    void validate_sorted_theory();

    bool operator==(const Theory &that) const;
    bool operator!=(const Theory &that) const;

    /**
     * @returns Inverse to parse_expr
     */
    std::string print(const Expr &e) const;
    std::string print();
    std::string print(const SortDecl &x) const;
    std::string print(const OpDecl &x) const;
    std::string print(const Rule &x, const int &dir = -1) const;

    /**
     * @returns Max arity of any sort constructor or operation
     */
    int max_arity() const;

    /**
     *  Elaborate type information of expression using the theory
     */
    Expr upgrade(const Expr &e) const;

    /**
     * Elaborate type information of constituent declarations / rules
     */
    Theory upgrade() const;

    Ve parse_exprs(const std::string &pth) const;
    Expr parse_expr(const std::string &expr) const;
    static Theory parseTheory(const std::string pth);

    std::map<std::string, int> symcode() const;

private:
    void validate_theory();
    SortDeclDict make_sdict(std::vector<SortDecl> s);
    OpDeclDict make_odict(std::vector<OpDecl> o);

    static std::string strParser(const std::string &pat);
    std::string mkParser() const;
    Expr ast_to_expr(const std::shared_ptr<peg::Ast> &ast) const;

    static SortDecl parseSort(std::shared_ptr<peg::Ast> ast, KindDict kd);
    static OpDecl parseOp(std::shared_ptr<peg::Ast> ast, KindDict kd);
    static Rule parseRule(std::shared_ptr<peg::Ast> ast, KindDict kd);
    static Expr parseExpr(std::shared_ptr<peg::Ast> ast, KindDict kd);
};

/**
 * Simple constructor for sort exprs
 * @param sym Sort symbol
 * @param args Default empty list of args
 * @returns an Expr with SortNode kind
 */
Expr Srt(const std::string &sym, const Ve &args = Ve{});
/**
 * Simple constructor for operator application exprs
 * @param sym Operator symbol
 * @param args Default empty list of args
 * @returns an Expr with AppNode kind
 */
Expr App(const std::string &sym, const Ve &args = Ve{});
/**
 * Simple constructor for variable exprs
 * @param sym Variable symbol
 * @param srt Sort expression
 * @returns an Expr with VarNode kind
 */
Expr Var(const std::string &sym, Expr srt);

template <typename T>

/**
 * Join list of printable eelements with an optional separator
 *
 * @param v vector of printable things (e.g. ["a","b","c"])
 * @param sep separator (e.g. ",")
 * @returns joined string (e.g. "a,b,c")
 */
std::string join(const std::vector<T> &v, const std::string &sep = "");

Vs split(const std::string &s, const std::string &delim);

/**
 * Remove first and last character
 *
 * @param x string to be trimmed
 * @returns trimmed string
 */
std::string trim(const std::string &x);

bool isSpace(unsigned char c);

void del_whitespace(std::string s);

#endif