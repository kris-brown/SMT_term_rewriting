# SMT_term_rewriting

Generalized algebraic theories (GATs) consist of:
- declaring types of entities (called sorts, which can be dependent on values, like "List of length `i`" for some `i::Int`),
- operator declarations (which can be thought of as functions among terms inhabiting the above sorts)
- equations (notions of when two sorts or two terms are equal).

This is a very expressive language, but to be useful we must be able to identify when arbitrary terms are equal or not with respect to the equations of the theory. Although this is an undecidable problem in general, we can convert the question of whether two terms are equal into a logic problem which can be solved by a SMT solver through finite model checking. This strategy has at least two caveats: we are restricted to checking whether a path of rewrites exists up to a finite length, and we can apply rewrites only up to a finite depth from the root of any term.

## Source code structure
- `ast.hpp`
    - The main program which takes a user-specified theory and creates a CVC4 model.
- `astextra.hpp`
    - The majority of the code, aimed at taking a GAT and generating relevant CVC4 terms/functions.
- `cvc4extra.hpp`
    - A few utility functions that are purely related to CVC4.
- `theory.hpp`
    - Datastructures for GATs and expressions, algorithms for substitution/pattern matching/validating/etc.
- `theories/theories.hpp`
    - It `#include`s the GATs in this folder and provides a means of iterating through them.

## TO-DO
 - Add more tests to make sure the generated CVC4 functions behave as expected
 - Connect to [SMT-switch](https://github.com/makaimann/smt-switch) and [cosa2](https://github.com/upscale-project/cosa2) to do model checking.
 - Handle the introduction free variables via rewrite rules. This has been done already in the [Julia version of this project](https://kris-brown.github.io/AlgebraicTypeTheory.jl/dev/), which generates a large CVC4 native input file string rather than using an API.
 - Cleaning things up and more documentation.

## Usage

GATs can be declared in two ways. Firstly, they can be constructed with a C++ API, with examples in the `src/theories` folder. To make these accessible, an additional line must be added to the `alltheories` function in `src/theories/theories.hpp`. However, at runtime it's also possible to point to a file with a GAT. Each theory currently in `src/theories` has an equivalent model data file in the `data` folder. This is an snippet of the theory of arrays, to which we can read and write objects:
```
Sort Ob "Ob" "Some datatype that can be stored in an array" []
Sort N "Nat" "A natural number: 0, 1, 2, ..." []
Sort B "Bool" "Either true or false" []
Sort Arr "Arr" "An array that can be indexed by natural numbers." []

Op S "S({})" "Successor, e.g. S(0)=1" I [i:N]
Op E "({}≡{})" "Equality of numbers" B [i:N, j:N]
Op ite "ite({},{},{})" "If-then-else" Ob [b:B, o:Ob, p:Ob]
Op read "read({},{})" "Read array A at position i" Ob [A:Arr, i:N]
Op write "write({},{},{}) "Write object o to position i" Arr [A:Arr, i:N, o:Ob]

Rule row "Read-over-write: if indices not equal, look at previous write"
    read(write(A:Arr, i:N, o:Ob), j:N)
    ite(E(i:N, j:N), o:Ob, read(A:Arr,j:N))
Rule eq1 "Recursively peel of successor"
    E(i:N,j:N)
    E(S(i:N),S(j:N))
```

As an example with sorts that depend on values, consider the theory of categories:
```
Sort Ob "Ob" "Objects in a category" []
Sort Hom "({}⇒{})" "Hom-set of morphisms" [A:Ob, B:Ob]

Op id "id({})" "Identity morphism" Hom(A:Ob, A:Ob) [A:Ob]
Op cmp "({} ⋅ {})" "Composition of morphisms" Hom(A:Ob, C:Ob) [f:Hom(A:Ob,B:Ob), g:Hom(B:Ob, C:Ob)]
```

The second argument for Sort/Operation declarations is important for both printing and parsing expressions of the GAT. Parsing is important because the main program requires a path to file with pairs of expressions to be tested for equality, for example `data/arrayterms.dat`.

To run this, first install [CVC4](https://github.com/CVC4/CVC4), then compile the program `src/ast.cpp` (this works on my Mac by running `src/bin/build.sh`). Run the executable with the name of the theory (or a path to a theory file) as a command line argument, e.g. `./build/ast natarray data/natarray1.dat`, where the second argument points to the expressions tested for equality (optionally add a integer argument to specify path depth).

