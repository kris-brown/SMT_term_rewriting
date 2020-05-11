#!/bin/bash

x=$(find src -name "*.cpp" -not -path "src/ast.cpp")

c++ -std=c++11 $(for i in $x; do echo "$i".o; done) test/test.cpp -o build/runtest -Wl,-rpath,/usr/local/lib /usr/local/lib/libcvc4parser.6.dylib /usr/local/lib/libcvc4.6.dylib /usr/local/lib/libgmp.dylib
