#!/bin/bash

# Create object files
x=$(find src -name "*.cpp")

for i in $x; do
    c++ -g -O0 -isystem /usr/local/include  -I$(pwd)  -Wno-c++17-extensions -Wall -std=gnu++11 -o  $i.o -c $i
done

# Create executable
c++ -g -O0 -Wall -Wl,-search_paths_first -Wl,-headerpad_max_install_names -L/usr/local/opt/llvm/lib $(for i in $x; do echo "$i".o; done) -o build/ast -Wl,-rpath,/usr/local/lib /usr/local/lib/libcvc4parser.6.dylib /usr/local/lib/libcvc4.6.dylib /usr/local/lib/libgmp.dylib


