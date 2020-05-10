# SMT_term_rewriting

To run this, first install [CVC4](https://github.com/CVC4/CVC4), then compile the program `src/ast.cpp` (this works on my Mac by running `src/bin/build.sh`)

Algebraic theories can be added to the `src/theories` folder, along with a corresponding additional line to the `get_theory` function in `src/theories/theories.hpp`.

Run the executable with the name of the theory as a sole command line argument, e.g. `./build/ast intarray`.