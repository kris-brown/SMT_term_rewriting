#ifndef THEORY_PREORDER
#define THEORY_PREORDER

#include "../theory.hpp"

// https://en.wikipedia.org/wiki/Preorder

Theory preorder()
{
    Expr Ob = Srt("Ob"), A = Var("A", Ob), B = Var("B", Ob), C = Var("C", Ob);
    Expr aa = Srt("L", {A, A}), ab = Srt("L", {A, B}), bc = Srt("L", {B, C}), ac = Srt("L", {A, C});
    Expr p = Var("p", ab), q = Var("q", bc), r = Var("r", ab);
    SortDecl dOb{"Ob", "Ob", {}, "Some set"};
    SortDecl dLeq{"L", "({}≤{})", {A, B}, "A relation from A -> B"};
    OpDecl refl{"R", "{}ᵣ", aa, {A}, "Reflexivity"};
    OpDecl trans{"trans", "({}⪯{})", ac, {p, q}, "Transitivity"};
    Rule singl{"Singleton", "The sort A->B is a singleton set.", p, r};
    return {"preorder", std::vector<SortDecl>{dOb, dLeq}, {refl, trans}, {singl}};
}

#endif