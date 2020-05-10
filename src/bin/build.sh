#!/bin/bash

/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   -isystem /usr/local/include  -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk -mmacosx-version-min=10.14 -I$(pwd)  -Wno-c++17-extensions -Wall -std=gnu++11 -o  src/theory.cpp.o -c src/theory.cpp

/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   -isystem /usr/local/include  -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk -mmacosx-version-min=10.14 -I$(pwd)  -Wno-c++17-extensions -Wall -std=gnu++11 -o   src/theories/cat.cpp.o -c src/theories/cat.cpp

/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   -isystem /usr/local/include  -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk -mmacosx-version-min=10.14 -I$(pwd)  -Wno-c++17-extensions -Wall -std=gnu++11 -o   src/theories/intarray.cpp.o -c src/theories/intarray.cpp


/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   -isystem /usr/local/include  -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk -mmacosx-version-min=10.14 -I$(pwd)  -Wno-c++17-extensions -Wall -std=gnu++11 -o  src/cvc4extra.cpp.o -c src/cvc4extra.cpp

/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   -isystem /usr/local/include  -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk -mmacosx-version-min=10.14 -I$(pwd)  -Wno-c++17-extensions -Wall -std=gnu++11 -o  src/astextra.cpp.o -c src/astextra.cpp


/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   -isystem /usr/local/include  -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk -mmacosx-version-min=10.14 -I$(pwd)  -Wno-c++17-extensions -Wall -std=gnu++11 -o  src/ast.cpp.o -c src/ast.cpp

echo "NO ERRORS YET"



###############
/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk -mmacosx-version-min=10.14 -Wall -Wl,-search_paths_first -Wl,-headerpad_max_install_names -L/usr/local/opt/llvm/lib src/theories/cat.cpp.o src/theories/intarray.cpp.o src/theory.cpp.o src/CVC4extra.cpp.o src/astextra.cpp.o src/ast.cpp.o  -o build/ast -Wl,-rpath,/usr/local/lib /usr/local/lib/libcvc4parser.6.dylib /usr/local/lib/libcvc4.6.dylib /usr/local/lib/libgmp.dylib /Users/ksb/CVC4/deps/install/lib/libantlr3c.a

echo "ERRORS"

# rm ast.cpp.o
# rm theory.cpp.o
# rm cvc4extra.cpp.o
# rm astextra.cpp.o
# rm cat.cpp.o
# rm intarray.cpp.o


