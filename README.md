# SMT_term_rewriting

To run this, first install [CVC4](https://github.com/CVC4/CVC4), then compile the program `src/ast.cpp` (this works on my Mac by running `src/bin/build.sh`).

Generalized algebraic theories (GATs) consist of sort declarations (which can be dependent types), operator declarations, and equations (between sorts or terms). This is a very expressive language, but to be useful we must be able to identify when terms are equal or not equal modulo the equations of the theory. Although this is an undecidable problem in general, we can convert the question of whether two terms are equal into a logic problem which can be solved by a SMT solver. This strategy has at least two caveats: we are restricted to checking whether a path of rewrites exists up to a finite length, and we can apply rewrites only up to a finite depth from the root of any term.


These GATs can be added to the `src/theories` folder, along with a corresponding additional line to the `get_theory` function in `src/theories/theories.hpp`.


Run the executable with the name of the theory as a sole command line argument, e.g. `./build/ast intarray`.


Tests involving CVC4 models will write the model to `build/test` for inspection.