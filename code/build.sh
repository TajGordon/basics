#!/bin/bash

cflags="-Wall"
includes="-I/opt/homebrew/Cellar/raylib/5.0/include"
libs="-lraylib"

output="-ogame"

cc $includes src/main.c $output $libs $cflags $warnings
