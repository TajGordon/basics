#!/bin/bash

cflags="-std=c++20 -Wall"
includes="-I/opt/homebrew/Cellar/raylib/5.0/include"
libs="-lraylib"

output="-ogame"

c++ $includes src/main.cpp $output $libs $cflags $warnings
