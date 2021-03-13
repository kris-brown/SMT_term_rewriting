#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>

#include "theory.hpp"

// Safe constructor
Expr::Expr(const std::string s,
           const Expr::NodeType k,
           const std::vector<Expr> a) : sym(s), kind(k), args(a)
{
    validate_expr();
}

void Expr::validate_expr()
{
    if (sym.empty())
        throw std::runtime_error("Expr is empty");
    else if (kind == Expr::VarNode)
    {
        if (args.size() != 1)
            throw std::runtime_error("Var '" + sym + "' has " + std::to_string(args.size()) + " != 1 args");
        else if (args.at(0).kind != Expr::SortNode)
            throw std::runtime_error("Var arg is not sort");
    }
    else if (kind == Expr::AppNode)
    {
        for (int i = 0; i != args.size(); i++)
        {
            if (i > 0 && args.at(i).kind == Expr::SortNode)
                throw std::runtime_error("Operators cannot operate on types");
        }
    }
    else if (kind == Expr::SortNode)
    {
        for (auto &&arg : args)
        {
            if (arg.kind == Expr::SortNode)
                throw std::runtime_error("Sorts cannot operate on sorts");
        }
    }
}

bool Expr::operator==(const Expr &that) const
{
    return sym == that.sym && kind == that.kind && args == that.args;
}

bool Expr::operator!=(const Expr &that) const
{
    return !(*this == that);
}

bool SortDecl::operator==(const SortDecl &that) const
{
    return sym == that.sym && pat == that.pat && args == that.args && desc == that.desc;
}

bool SortDecl::operator!=(SortDecl const &that) const
{
    return !(*this == that);
}

bool OpDecl::operator==(const OpDecl &that) const
{
    return sym == that.sym && pat == that.pat && sort == that.sort && args == that.args && desc == that.desc;
}

bool OpDecl::operator!=(OpDecl const &that) const
{
    return !(*this == that);
}

bool Rule::operator==(const Rule &that) const
{
    return name == that.name && t1 == that.t1 && t2 == that.t2 && desc == that.desc;
}

bool Rule::operator!=(Rule const &that) const
{
    return !(*this == that);
}

bool Theory::operator==(const Theory &that) const
{
    return name == that.name && sorts == that.sorts && ops == that.ops && rules == that.rules;
}
bool Theory::operator!=(const Theory &that) const
{
    return !(*this == that);
}

std::string Theory::print(const Expr &e) const
{

    if (e.kind == Expr::VarNode)
        return e.sym + ":" + print(e.args.at(0));
    else
    {
        Expr::NodeType firstchild = e.args.size() ? e.args.at(0).kind : Expr::VarNode;
        bool sorted_app = (firstchild == Expr::SortNode) && (e.kind == Expr::AppNode);
        Vs children;

        for (int i = 0; i != e.args.size(); i++)
        {
            if ((i != 0) || (!sorted_app))
            {
                children.push_back(print(e.args.at(i)));
            }
        }

        std::string pat = ((e.kind == Expr::SortNode) ? sorts.at(e.sym).pat : ops.at(e.sym).pat);
        Vs syms = split(pat, "{}");
        std::string ret;
        for (int i = 0; i != children.size(); i++)
            ret += syms.at(i) + children.at(i);
        return ret + syms.back();
    }
}

// Pretty print
std::string Theory::print(const SortDecl &x) const
{
    Vs args;
    for (auto &&a : x.args)
        args.push_back(print(a.uninfer()));
    return "Sort: " + x.sym + " " + x.pat + "\n\t" + join(args, "\n\t");
}

// Pretty print
std::string Theory::print(const OpDecl &x) const
{
    Vs args;
    for (auto &&a : x.args)
        args.push_back(print(a.uninfer()));
    return "Op: " + x.sym + " " + x.pat + "\n\t" + join(args, "\n\t");
}

// Pretty print
std::string Theory::print(const Rule &x, const int &dir) const
{
    std::string p1 = "\t", p2 = "\t";
    switch (dir)
    {
    case 1:
        p1 += "⟵\t";
        p2 += "⟶\t";
        break;
    case 0:
        p2 += "⟵\t";
        p1 += "⟶\t";
        break;
    }
    return "Rule: " + x.name + "\n" + p1 + print(x.t1.uninfer()) + "\n" + p2 + print(x.t2.uninfer());
}

// Pretty print
std::string Theory::print(){
    Vs lines;
    for (auto &&[v, s] : sorts)
        lines.push_back(print(s));
    for (auto &&[v, s] : ops)
        lines.push_back(print(s));
    for (auto &&r : rules)
        lines.push_back(print(r));

    return name + "\n\n" + join(lines, "\n");
}

std::ostream &operator<<(std::ostream &out, const Expr &e)
{
    if (e.kind == Expr::VarNode)
        out << e.sym << ":" << e.args.at(0);
    else
    {
        Expr::NodeType firstchild = e.args.size() ? e.args.at(0).kind : Expr::VarNode;
        bool sorted_app = (firstchild == Expr::SortNode) && (e.kind == Expr::AppNode);
        if (sorted_app && e.args.size() > 1)
            out << "(";
        out << e.sym;
        if (e.args.size() > (sorted_app ? 1 : 0))
        {
            out << "(";
            for (int i = 0; i != e.args.size(); i++)
            {
                if (i > 0 || !sorted_app)
                    out << e.args.at(i) << ",";
            }
            out << "\b)";
        }
        if (sorted_app)
        {
            if (e.args.size() > (sorted_app ? 1 : 0))
                out << ")";
            out << "::" << e.args.at(0);
        }
    }
    return out;
}

// Create dictionary with symbols as keys
SortDeclDict Theory::make_sdict(std::vector<SortDecl> s)
{
    SortDeclDict sdict;
    for (auto &&sd : s)
        sdict.insert({sd.sym, sd});
    return sdict;
}

// Create dictionary with symbols as keys
OpDeclDict Theory::make_odict(std::vector<OpDecl> o)
{
    OpDeclDict odict;
    for (auto &&od : o)
        odict.insert({od.sym, od});
    return odict;
}

Theory::Theory() : name("Default"), sorts({}), ops({}), rules({}) {}

Theory::Theory(const std::string n,
               const std::vector<SortDecl> s,
               const std::vector<OpDecl> o,
               const std::vector<Rule> r) : name(n),
                                            sorts(make_sdict(s)),
                                            ops(make_odict(o)),
                                            rules(r)
{
    validate_theory();
}

Theory::Theory(const std::string n,
               const SortDeclDict s,
               const OpDeclDict o,
               const std::vector<Rule> r) : name(n), sorts(s), ops(o), rules(r) { validate_theory(); }

// Create AST with PEGlib, then parse it
Theory Theory::parseTheory(const std::string pth)
{
    peg::parser parser(R"(
Items <- WORD Item*
Item <- SortDecl / OpDecl / Rule
SortDecl <- 'Sort' WORD PHRASE PHRASE '[' Term* ']'
OpDecl <- 'Op' WORD PHRASE PHRASE Term '[' Term* ']'
Rule <- 'Rule' WORD PHRASE Term Term
Term <- Var / WORD '(' Term* ')' / WORD / '[[' Term '|' Term ']]'
Var <- WORD ':' Term
WORD <- < [a-zA-Z_] [a-zA-Z0-9_]* >
PHRASE <- < '"' (!'"' .)* '"' >
%whitespace  <-  [ \t\r\n,]*
)");
    // Confirm PEGlib parsed the parser correctly
    assert((bool)parser == true);
    parser.enable_ast();
    parser.enable_packrat_parsing();

    // Validate path to file containing the GAT
    std::ifstream infile(pth);
    if (infile.fail())
    {
        infile.close();
        throw std::runtime_error("Bad path to theory file: " + pth);
    }

    // Feed the file to the parser and process the AST
    std::string content((std::istreambuf_iterator<char>(infile)),
                        (std::istreambuf_iterator<char>()));
    std::shared_ptr<peg::Ast> ast;
    if (parser.parse(content.c_str(), ast))
    {
        ast = peg::AstOptimizer(true).optimize(ast);

        // Parse the theory name
        std::string name = ast->nodes.at(0)->token;

        // Top-level scan of AST to initialize mapping of symbols to
        // whether they should be parsed as sorts or operators.
        KindDict kinddict;

        for (int i = 1; i < ast->nodes.size(); i++)
        {
            std::shared_ptr<peg::Ast> item = ast->nodes.at(i);
            if (item->name == "SortDecl")
            { // Terms with this symbol should be parsed as sorts
                kinddict.insert({item->nodes.at(0)->token, Expr::SortNode});
            }
            if (item->name == "OpDecl")
            { // Terms with this symbol should be parsed as applications
                kinddict.insert({item->nodes.at(0)->token, Expr::AppNode});
            }
        }

        // initialize theory data
        std::vector<SortDecl> sorts;
        std::vector<OpDecl> ops;
        std::vector<Rule> rules;

        // Parse the top-level theory data
        for (int i = 1; i < ast->nodes.size(); i++)
        {
            std::shared_ptr<peg::Ast> item = ast->nodes.at(i);
            if (item->name == "SortDecl")
                sorts.push_back(parseSort(item, kinddict));
            else if (item->name == "OpDecl")
                ops.push_back(parseOp(item, kinddict));
            else if (item->name == "Rule")
                rules.push_back(parseRule(item, kinddict));
            else
            {
                throw std::runtime_error("Bad theory file: " + pth + " has top-level " + (item->name));
            }
        }
        return {name, sorts, ops, rules};
    }
    else
    {
        throw std::runtime_error("syntax error in theory file:" + pth);
    }
}

Expr Theory::parseExpr(std::shared_ptr<peg::Ast> ast, KindDict kd)
{
    std::string name;
    Expr::NodeType nt;
    Ve args;

    // Handle a term with a manual type annotation specially
    if (ast->choice == 3) {
        assert(ast->nodes.size()==2);
        Expr trm = parseExpr(ast->nodes.at(0), kd);
        Expr typ = parseExpr(ast->nodes.at(1), kd);
        Ve newargs={typ};
        for (auto &&e : trm.args)
            newargs.push_back(e);
        return {trm.sym, trm.kind, newargs};
    }



    if (ast->nodes.empty())
        name = ast->token; // no argument case
    else
    {
        // First arg is the symbol, others should be recursively parsed
        for (int i = 0; i != ast->nodes.size(); i++)
        {
            if (i == 0)
                name = ast->nodes.at(i)->token;
            else
                args.push_back(parseExpr(ast->nodes.at(i), kd));
        }
    }

    // Assume unknown symbols are variables
    if (kd.find(name) == kd.end())
        nt = Expr::VarNode;
    else if (kd.at(name) == Expr::AppNode)
        nt = Expr::AppNode;
    else
        nt = Expr::SortNode;

    Expr out{name, nt, args}; // validate_expr

    return out;
}

SortDecl Theory::parseSort(std::shared_ptr<peg::Ast> ast, KindDict kd)
{
    Ve args;
    for (int i = 3; i != ast->nodes.size(); i++)
        args.push_back(parseExpr(ast->nodes.at(i), kd));

    return {ast->nodes.at(0)->token,
            trim(ast->nodes.at(1)->token),
            args, trim(ast->nodes.at(2)->token)};
}

OpDecl Theory::parseOp(std::shared_ptr<peg::Ast> ast, KindDict kd)
{
    Ve args;
    for (int i = 4; i != ast->nodes.size(); i++)
        args.push_back(parseExpr(ast->nodes.at(i), kd));
    return {ast->nodes.at(0)->token,
            trim(ast->nodes.at(1)->token),
            parseExpr(ast->nodes.at(3), kd),
            args, trim(ast->nodes.at(2)->token)};
}
Rule Theory::parseRule(std::shared_ptr<peg::Ast> ast, KindDict kd)
{
    return {ast->nodes.at(0)->token,
            trim(ast->nodes.at(1)->token),
            parseExpr(ast->nodes.at(2), kd),
            parseExpr(ast->nodes.at(3), kd)};
}

void Theory::validate_theory()
{
    // TBD
}

void Theory::validate_sorted_theory()
{
    // TBD
}

Expr Srt(const std::string &sym, const Ve &args)
{
    return Expr{sym, Expr::SortNode, args};
}
Expr App(const std::string &sym, const Ve &args)
{
    return Expr{sym, Expr::AppNode, args};
}
Expr Var(const std::string &sym, Expr srt)
{
    return Expr{sym, Expr::VarNode, {srt}};
}

template <typename T>
std::string join(const std::vector<T> &v, const std::string &sep)
{
    std::stringstream ss;

    for (size_t i = 0; i < v.size(); ++i)
    {
        if (i != 0)
            ss << sep;
        ss << v[i];
    }
    std::string s = ss.str();
    return s;
}

std::string trim(const std::string &x)
{
    return x.substr(1, x.length() - 2);
}

std::map<Vi, size_t> Expr::gethash() const
{
    // Build up result mapping recursively
    std::map<Vi, size_t> result;

    Vi arghashes; // Hashes of one's immediate arguments
    for (int i = 0; i != args.size(); i++)
    {
        for (auto &&[subpath, sp_hash] : args[i].gethash())
        {
            if (subpath.empty())
                arghashes.push_back(sp_hash);
            Vi newk = subpath;
            newk.insert(newk.begin(), i);
            result[newk] = sp_hash;
        }
    }
    // Create hash by hashing the argument hashes
    size_t curr = std::hash<std::string>()(sym + join(arghashes, "|"));

    result[Vi()] = curr; // Append to the result

    return result;
}

std::map<size_t, Vvi> Expr::distinct(const std::map<Vi, size_t> &hashes)
{
    std::map<size_t, Vvi> result;
    for (auto &&[subpath, hashval] : hashes)
    {
        if (result.find(hashval) == result.end())
            result[hashval] = {subpath}; // first time seeing this term
        else
            result[hashval].push_back(subpath);
    }
    return result;
}

Expr Expr::subexpr(const Vi &pth) const
{
    const Expr copy = Expr(*this);
    std::vector<Expr> res{copy}; // initial condition
    for (auto i : pth)           // keep taking the i'th argument, building up a list of terms
        res.emplace_back(res.back().args.at(i));
    return res.back(); // The final term of this list is the one we want
}

void Expr::addx(std::set<std::string> &syms, const int &node_type) const
{
    if (node_type < 0 || kind == node_type)
        syms.insert(sym);
    for (auto &&e : args)
        e.addx(syms, node_type);
}

// Encode each symbol mentioned in a theory as a unique integer
std::map<std::string, int> Theory::symcode() const
{
    // Grab all the symbols mentioned anywhere
    std::set<std::string> syms;
    for (auto &&[_, v] : sorts)
    {
        for (auto &&e : v.args)
            e.addx(syms);
    }
    for (auto &&[_, v] : ops)
    {
        v.sort.addx(syms);
        for (auto &&e : v.args)
            e.addx(syms);
    }
    for (auto &&r : rules)
    {
        r.t1.addx(syms);
        r.t2.addx(syms);
    }
    // Give each a unique number starting at 1
    std::map<std::string, int> result;
    int i = 1;
    for (auto str : syms)
        result[str] = i++;
    return result;
}

// If failure, return dict with "" key.
void Expr::mergedict(MatchDict &acc, const MatchDict &m)
{
    for (auto &&[k, v] : m)
    {
        if (acc.find(k) == acc.end())
            acc.insert({k, v}); // NEW KEY
        else if (acc.at(k) != v)
        {
            acc.insert({"", v}); // CONFLICT
        }
    }
}

MatchDict Expr::patmatch(const Expr &expr) const
{
    MatchDict result;

    if (kind == Expr::VarNode)
    {
        // In case of variable, the whole expr matches
        result.insert({sym, expr});

        // Also match the sort of the variable to type of expr, which is 1st arg
        assert(!expr.args.empty());
        assert(args.size() == 1);
        Expr exprSort = expr.args.at(0);
        assert(exprSort.kind == SortNode);
        mergedict(result, args.at(0).patmatch(exprSort));

        return result;
    }
    else if (expr.sym != sym || expr.args.size() != args.size())
        return {{"", expr}}; // error match result
    else
    {
        // Recurse on arguments if top-level constant symbol agrees
        for (int i = 0; i != expr.args.size(); i++)
            mergedict(result, args.at(i).patmatch(expr.args.at(i)));

        return result;
    }
}

Expr Expr::sub(const MatchDict &matchdict) const
{
    if (matchdict.find(sym) != matchdict.end())
    {
        assert(kind == VarNode);
        return matchdict.at(sym);
    }
    else
    {
        Ve newargs;
        for (auto &&arg : args)
            newargs.push_back(arg.sub(matchdict));
        return {sym, kind, newargs};
    }
}

bool Expr::inferred() const
 {
     if (kind==AppNode) {
         return !(args.empty()) && (args.at(0).kind == SortNode);
     } else {
         return true;
     }
 }

Expr Expr::infer(const SortDeclDict &sorts,
                 const OpDeclDict &ops,
                 const std::string &sym,
                 const Ve &args)
{

    // Handle error of unknown operator
    if (ops.find(sym) == ops.end())
        throw std::runtime_error("inferring a symbol (" + sym + ")not in ops");

    // Handle operators mistakenly operating on sorts
    for (auto &&a : args)
    {
        if (a.kind == Expr::SortNode)
            throw std::runtime_error("inferring a term that has a Sort as a direct argument: is this term already inferred?");
    }

    Ve op_pat_args = ops.at(sym).args;

    // Handle error of incorrect # of args
    if (op_pat_args.size() != args.size())
    {
        std::stringstream buf;
        buf << args.size() << " inferred args" << std::endl;
        buf << op_pat_args.size() << " args in pat " << sym << std::endl;
        for (auto &&a : op_pat_args)
            buf << "\t" << a << std::endl;
        throw std::runtime_error(buf.str());
    }

    // Determine relation between the actual arguments and the canonical arguments.
    // Store results in a dictionary binding variables to Exprs.
    MatchDict match;
    for (int i = 0; i != op_pat_args.size(); i++)
    {
        Expr x = args.at(i), pat = op_pat_args.at(i);
        MatchDict newmatch = pat.patmatch(x);
        if (newmatch.find("") != newmatch.end())
        {
            std::stringstream buf;
            buf << sym << " pattern match fail for inferring arg " << i << std::endl;
            buf << "Arg is " << x << std::endl
                << "pat is " << pat << std::endl;
            throw std::runtime_error(buf.str());
        }
        mergedict(match, newmatch);

        if (match.find("") != match.end())
        {
            std::stringstream buf;
            buf << sym << " pattern match fail given conflicting args after adding arg " << i << std::endl;
            for (auto &&a : args)
                buf << "\n\t" << a << std::endl;
            throw std::runtime_error(buf.str());
        }
    }
    // Result sort is just a substitution of the canonical result sort
    Expr res=ops.at(sym).sort.sub(match);
     //upgrade because the op return type pattern may contain a function which needs its
     // type to be inferred
    return res.upgrade(sorts, ops);
}

// Elaborate type information by recursively calling infer
Expr Expr::upgrade(const SortDeclDict &sorts,
                   const OpDeclDict &ops) const
{
    // Avoid upgrading something that has already been upgraded
    if (this->kind==AppNode && !this->args.empty() && this->args.at(0).kind ==SortNode) {
        Ve recargs = {this->args.at(0)};
        for (int i = 1; i != args.size(); i++)
            {recargs.push_back(this->args.at(i).upgrade(sorts, ops));}
        return {this->sym, this->kind, recargs};
    }
    Ve recargs, newargs;
    for (auto &&a : args)
        recargs.push_back(a.upgrade(sorts, ops));
    if (kind == Expr::AppNode)
        {newargs.push_back(infer(sorts, ops, sym, recargs));}
    for (auto &&a : recargs)
        newargs.push_back(a);

    Expr res{sym, kind, newargs};
    return res;
}

// Elaborate type information
Expr Theory::upgrade(const Expr &e) const
{
    return e.upgrade(sorts, ops);
}

// Elaborate type information
SortDecl SortDecl::upgrade(const SortDeclDict &sorts,
                           const OpDeclDict &ops) const
{
    Ve newargs;
    for (auto &&a : args)
        newargs.push_back(a.upgrade(sorts, ops));
    return {sym, pat, newargs, desc};
}

// Elaborate type information
OpDecl OpDecl::upgrade(const SortDeclDict &sorts,
                       const OpDeclDict &ops) const
{
    Ve newargs;
    for (auto &&a : args)
        newargs.push_back(a.upgrade(sorts, ops));
    return {sym, pat, sort.upgrade(sorts, ops), newargs, desc};
}

// Elaborate type information
Rule Rule::upgrade(const SortDeclDict &sorts,
                   const OpDeclDict &ops) const
{
    return {name, desc, t1.upgrade(sorts, ops), t2.upgrade(sorts, ops)};
}

// Elaborate type information
Theory Theory::upgrade() const
{
    SortDeclDict newsorts;
    OpDeclDict newops;
    std::vector<Rule> newrules;
    // TODO: In general, we should topologically sort the sorts and operations
    for (auto &&[k, v] : sorts)
    {
        newsorts.insert(std::pair<std::string, SortDecl>(k, v.upgrade(sorts, ops)));
    }
    for (auto &&[k, v] : ops)
    {
        newops.insert(std::pair<std::string, OpDecl>(k, v.upgrade(sorts, ops)));
    }
    for (auto &&r : rules)
    {
        newrules.push_back(r.upgrade(sorts, ops));
    }
    Theory *t2 = new Theory(name, newsorts, newops, newrules);
    // t2.validate_sorted_theory();
    return *t2;
}

int Theory::max_arity() const
{
    int m = 1;
    for (auto &&[k, v] : sorts)
    {
        if (v.args.size() > m)
            m = v.args.size();
    }
    for (auto &&[k, v] : ops)
    {
        if (v.args.size() > m)
            m = v.args.size();
    }
    return m;
}

Vs split(const std::string &s, const std::string &delim)
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

bool isSpace(unsigned char c)
{
    return (c == ' ' || c == '\n' || c == '\r' ||
            c == '\t' || c == '\v' || c == '\f');
}

void del_whitespace(std::string s)
{
    s.erase(std::remove_if(s.begin(), s.end(), isSpace), s.end());
}

std::string Theory::strParser(const std::string &pat)
{
    std::stringstream ss;
    Vs raw = split(pat, "{}");
    for (int i = 0; i != raw.size() - 1; i++)
    {
        std::string s = raw.at(i);
        std::regex r("\\s+");
        s = std::regex_replace(s, r, "");
        ss << "'" << s << "' Term ";
    }
    ss << "'" << raw.back() << "'\n";
    return ss.str();
}

std::string Theory::mkParser() const
{
    std::stringstream ss;
    ss << "Term <- Var";
    for (auto &&[k, v] : ops)
        ss << " / " << k;
    ss << "\nSort <- ";
    int i = 0;
    for (auto &&p : sorts)
    {
        if (i != 0)
            ss << " / ";
        ss << p.first;
        i++;
    }
    ss << '\n';
    for (auto &&[k, v] : sorts)
        ss << k << " <-  " << strParser(v.pat);
    ss << "\n";
    for (auto &&[k, v] : ops)
        ss << k << " <-  " << strParser(v.pat);
    ss << "Var <- WORD ':' Sort\nWORD <- < [a-zA-Z] [a-zA-Z0-9]* >\n%whitespace  <-  [ \\t\\r\\n]*";
    return ss.str();
}

Ve Theory::parse_exprs(const std::string &pth) const
{
    std::ifstream infile(pth);
    if (infile.fail())
    {
        infile.close();
        throw std::runtime_error("Bad path to file with expressions: " + pth);
    }
    Ve res;
    std::string line;
    while (std::getline(infile, line))
        res.push_back(parse_expr(line));
    infile.close();
    return res;
}

Expr Theory::parse_expr(const std::string &expr) const
{
    peg::parser parser(mkParser().c_str());
    parser.enable_ast();
    assert((bool)parser == true);

    std::shared_ptr<peg::Ast> ast;
    if (parser.parse(expr.c_str(), ast))
    {
        return ast_to_expr(ast);
    }
    else
    {
        throw std::runtime_error("syntax error: " + expr);
    }
}

Expr Theory::ast_to_expr(const std::shared_ptr<peg::Ast> &ast) const
{
    Expr res{"x", Expr::AppNode, {}};
    std::string kind = ast->name;

    if ((kind != "Term" && kind != "Sort") || ast->nodes.size() != 1)
        throw std::runtime_error("Bad AST. Name = " + ast->name);

    std::shared_ptr<peg::Ast> child = (ast->nodes).at(0);
    std::string name = child->name;

    if (name == "Var")
    {
        Ve srt{ast_to_expr(child->nodes.at(1))};
        return Expr(child->nodes.at(0)->token, Expr::VarNode, srt);
    }

    Ve children;
    for (auto node : child->nodes)
    {
        children.push_back(ast_to_expr(node));
    }

    if (kind == "Term")
    {
        return Expr(name, Expr::AppNode, children);
    }
    else if (kind == "Sort")
    {
        return Expr(name, Expr::SortNode, children);
    }
    else
    {
        throw std::runtime_error("Unknown symbol");
    }
}

std::map<std::string, int> Expr::freevar(const Expr &y) const
{
    std::set<std::string> symx, symy;
    Expr copy = Expr(*this);
    copy.addx(symx, Expr::VarNode);
    y.addx(symy, Expr::VarNode);

    std::map<std::string, int> res;
    int i = 1;
    for (auto &&e : symx)
    {
        if (symy.find(e) == symy.end())
            res[e] = i++;
    }
    return res;
}

Expr Expr::uninfer() const
{
    Ve newargs;
    for (auto &&a : args)
    {
        if ((kind != Expr::AppNode) || (a.kind != Expr::SortNode))
            newargs.push_back(a.uninfer());
    }
    return {sym, kind, newargs};
}
