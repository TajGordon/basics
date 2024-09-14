#!/bin/bash

cflags="-std=c++20 -Wall"
includes="-I/opt/homebrew/Cellar/raylib/5.0/include -I/opt/homebrew/Cellar/openssl@3/3.3.2/include"
libs="-lraylib -L/opt/homebrew/Cellar/openssl@3/3.3.2/lib -lssl -lcrypto"

output="-ogame"

c++ $includes src/main.cpp $output $libs $cflags $warnings
