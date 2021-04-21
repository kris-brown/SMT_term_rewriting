# Step 1

Clone [pono](https://github.com/upscale-project/pono) with [SMT-switch](https://github.com/makaimann/smt-switch) and [CVC4](https://github.com/CVC4/CVC4) support.

```
git clone https://github.com/upscale-project/pono.git
cd pono
./contrib/setup-smt-switch.sh
./contrib/setup-btor2tools.sh
./contrib/setup-bison.sh
./configure.sh --prefix=local
cd build
make
make install
```

# Step 2

Make the install locations available via environment variables. Supposing `pono` was installed to the directory `/some_path/`, a simple way to do this is to edit your `~/.bashrc` (then `source` it) with the following line:

```
export LD_LIBRARY_PATH=/some_path/pono/local/lib:/some_path/pono/deps/smt-switch/local/lib:$LD_LIBRARY_PATH
export LIBRARY_PATH=/some_path/pono/local/lib:/some_path/pono/deps/smt-switch/local/lib:$LIBRARY_PATH
export CPLUS_INCLUDE_PATH=/some_path/pono/local/include/pono:/some_path/pono/deps/smt-switch/local/include:/some_path/pono/deps/smt-switch/deps/CVC4/src/:/some_path/pono/deps/smt-switch/deps/CVC4/src/include:/some_path/pono/deps/smt-switch/deps/CVC4/build/src/:$CPLUS_INCLUDE_PATH
```



As an alternative to the above, you can manually add these paths with `-I` and `-L` flags to the `CPPFLAGS` and `LDFLAGS` variables (respectively) of the `Makefile`, which would avoid making global changes to your system.

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
