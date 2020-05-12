# SMT_term_rewriting

To run this, first install [CVC4](https://github.com/CVC4/CVC4), then compile the program `src/ast.cpp` (this works on my Mac by running `src/bin/build.sh`).

Generalized algebraic theories (GATs) consist of sort declarations (which can be dependent types), operator declarations, and equations (between sorts or terms). This is a very expressive language, but to be useful we must be able to identify when terms are equal or not equal modulo the equations of the theory. Although this is an undecidable problem in general, we can convert the question of whether two terms are equal into a logic problem which can be solved by a SMT solver through finite model checking. This strategy has at least two caveats: we are restricted to checking whether a path of rewrites exists up to a finite length, and we can apply rewrites only up to a finite depth from the root of any term.

Currently, a model is generated that has functions for:
 - Constructing expressions
 - Checking if expressions match a pattern for a rewrite rule
 - Rewriting (sub)expressions according to a rewrite rule
 - Testing whether a rewrite can be done in some fixed number of steps.

TO-DO:
 - Add more tests to make sure the generated CVC4 functions behave as expected
 - Connect to [SMT-switch](https://github.com/makaimann/smt-switch) to do model checking.
 - Re-implement functions related to introducing free variables via rewrite rules from the [Julia version of this project](https://kris-brown.github.io/AlgebraicTypeTheory.jl/dev/)


These GATs can be added to the `src/theories` folder, along with a corresponding additional line to the `get_theory` function in `src/theories/theories.hpp`.


Run the executable with the name of the theory as a command line argument, e.g. `./build/ast intarray` (optionally add a integer to specify path depth). This will produce a file `build/<theoryName>.dat`.


Tests involving CVC4 models will write the model to `build/test` for inspection. All current theories have tests in `test/theories_test.hpp`, so running the test suite (`test/test.cpp`) is tantamount to running the main program on all of those theories.