# Step 1
Clone [cosa2](https://github.com/upscale-project/cosa2) with [SMT-switch](https://github.com/makaimann/smt-switch) and [CVC4](https://github.com/CVC4/CVC4) support.
```
git clone https://github.com/upscale-project/cosa2.git
cd cosa2
./contrib/setup-smt-switch.sh --with-cvc4
./contrib/setup-btor2tools.sh
./contrib/setup-bison.sh
./configure.sh --prefix=local --with-cvc4
cd build
make
make install
```

# Step 2
Make the install locations available via environment variables. Supposing `cosa2` was installed to the directory `/some_path/`, a simple way to do this is to edit your `~/.bashrc` (then `source` it) with the following line:
```
export LD_LIBRARY_PATH=/some_path/cosa2/local/lib:/some_path/cosa2/deps/smt-switch/local/lib:$LD_LIBRARY_PATH
export LIBRARY_PATH=/some_path/cosa2/local/lib:/some_path/cosa2/deps/smt-switch/local/lib:$LIBRARY_PATH
export CPLUS_INCLUDE_PATH=/some_path/cosa2/local/include/cosa2:/some_path/cosa2/deps/smt-switch/local/include:$CPLUS_INCLUDE_PATH
```

As an alternative to the above, you can manually adding these paths with `-I` and `-L` flags to the `CPPFLAGS` and `LDFLAGS` variables (respectively) of the `Makefile` would avoid making global changes to your system.

# Step 3
Clone this project and build with the following commands:
```
git clone https://github.com/kris-brown/SMT_term_rewriting.git
cd SMT_term_rewriting
make all                  # compile the project
make test
build/runtest             # make sure the tests all pass
build/ast < data/inputs/1 # run an example
```
