# SMT_term_rewriting

Generalized algebraic theories (GATs) consist of:
- declaring types of entities (called sorts, which can depend on values, e.g. "List of length `i`"),
- operator declarations (which can be thought of as pure functions among terms inhabiting the above sorts)
- equations (notions of when two sorts or two terms are equal).

This is a very expressive language, but to be useful we must be able to identify when arbitrary terms are equal or not with respect to the equations of the theory. Although this is an undecidable problem in general, we can convert the question of whether two terms are equal into a logic problem which can be solved by a SMT solver through finite model checking. This strategy has at least two caveats: we are restricted to checking whether a path of rewrites exists up to a finite length, and we can apply rewrites only up to a finite depth from the root of any term.


## TO-DO
 - Reimplement CVC4 code with cosa2 in order to do model checking for an arbitrary (not pre-specified) number of rewrites.

## Usage

To run this, first install [CVC4](https://github.com/CVC4/CVC4) and [cosa2](https://github.com/upscale-project/cosa2), and add the install locations to `LD_LIBRARY_PATH`. Then the main program and tests can be built with `make all` and `make test` respectively.

The main executable prompts the user for the following inputs:
1. A user-specified theory (by name or path)
2. A starting term in that theory
3. A goal term in that theory
4. How many rewrite steps are needed
5. Max depth in the abstract syntax tree we want to be able to apply rewrites.

GATs can be declared in two ways. Firstly, they can be constructed with a C++ API, with examples in the `src/theories` folder. However, it's also possible to point to a file which specifies a GAT. Each theory currently in `src/theories` has an equivalent model data file in the `data` folder to show how this is done. This is a snippet of a theory of arrays (`data/natarray.dat`):
```
Sort Ob "Ob" "Some datatype that can be stored in an array" []
Sort N "Nat" "A natural number: 0, 1, 2, ..." []
Sort B "Bool" "Either true or false" []
Sort Arr "Arr" "An array that can be indexed by natural numbers." []

Op S "S({})" "Successor, e.g. S(0)=1" N [i:N]
Op E "({}≡{})" "Equality of numbers" B [i:N, j:N]
Op ite "ite({},{},{})" "If-then-else" Ob [b:B, o:Ob, p:Ob]
Op read "read({},{})" "Read array A at position i" Ob [A:Arr, i:N]
Op write "write({},{},{})" "Write object o to position i" Arr [A:Arr, i:N, o:Ob]

Rule row "Read-over-write: if indices not equal, look at previous write"
    read(write(A:Arr, i:N, o:Ob), j:N)
    ite(E(i:N, j:N), o:Ob, read(A:Arr,j:N))
Rule eq1 "Recursively peel off successor"
    E(i:N,j:N)
    E(S(i:N),S(j:N))
```

The second argument for Sort/Operation declarations is important for both printing and parsing expressions of the GAT. Parsing is important due to arguments 2 and 3, see for example `data/inputs1` or `data/inputs2`.

## Examples

Consider the example using the theory of categories in `data/inputs1` which searches for a rewrite path of length 2:
```
data/cat.dat
(x:(A:Ob⇒Q:Ob) ⋅ id(Q:Ob))
(id(A:Ob) ⋅ x:(A:Ob⇒Q:Ob))
2
3
```

The executable can be called with a file, e.g. `build/ast < data/inputs1`, which will print out the following results:

```
Solution found
Starting with:
        (x:(A:Ob⇒Q:Ob) ⋅ id(Q:Ob))
**************************************
Step 0: apply R1f (forward)
Rule: idl
        f:(A:Ob⇒B:Ob)
        (id(A:Ob) ⋅ f:(A:Ob⇒B:Ob))
at subpath Empty to yield:
        (id(A:Ob) ⋅ (x:(A:Ob⇒Q:Ob) ⋅ id(Q:Ob)))

**************************************
Step 1: apply R2r (reverse)
Rule: idr
        f:(A:Ob⇒B:Ob)
        (f:(A:Ob⇒B:Ob) ⋅ id(B:Ob))
at subpath P2 to yield:
        (id(A:Ob) ⋅ x:(A:Ob⇒Q:Ob))
````
It applies the left and right identity laws, though not in the order that a human would choose.

Likewise, for `data/inputs2` we model writing `p` to position 1 of some array, `o` to position 0, and then reading the value at position 1. We can prove that the result of this sequence of events is tantamount to just `p`:
```
Solution found
Starting with:
        read(write(write(A:Arr,S(0),p:Ob),0,o:Ob),S(0))
**************************************
Step 0: apply R1f (forward)
Rule: row
        read(write(A:Arr,i:Nat,o:Ob),j:Nat)
        ite((i:Nat≡j:Nat),o:Ob,read(A:Arr,j:Nat))
at subpath Empty to yield:
        ite((0≡S(0)),o:Ob,read(write(A:Arr,S(0),p:Ob),S(0)))

**************************************
Step 1: apply R4r (reverse)
Rule: eq3
        ⊥
        (0≡S(0))
at subpath P1 to yield:
        ite(⊥,o:Ob,read(write(A:Arr,S(0),p:Ob),S(0)))

**************************************
Step 2: apply R7r (reverse)
Rule: if2
        p:Ob
        ite(⊥,o:Ob,p:Ob)
at subpath Empty to yield:
        read(write(A:Arr,S(0),p:Ob),S(0))

**************************************
Step 3: apply R1f (forward)
Rule: row
        read(write(A:Arr,i:Nat,o:Ob),j:Nat)
        ite((i:Nat≡j:Nat),o:Ob,read(A:Arr,j:Nat))
at subpath Empty to yield:
        ite((S(0)≡S(0)),p:Ob,read(A:Arr,S(0)))

**************************************
Step 4: apply R2r (reverse)
Rule: eq1
        (i:Nat≡j:Nat)
        (S(i:Nat)≡S(j:Nat))
at subpath P1 to yield:
        ite((0≡0),p:Ob,read(A:Arr,S(0)))

**************************************
Step 5: apply R3r (reverse)
Rule: eq2
        ⊤
        (0≡0)
at subpath P1 to yield:
        ite(⊤,p:Ob,read(A:Arr,S(0)))

**************************************
Step 6: apply R6r (reverse)
Rule: if1
        o:Ob
        ite(⊤,o:Ob,p:Ob)
at subpath Empty to yield:
        p:Ob
```