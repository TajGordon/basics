cflags="-std=c++17"

if [[ "$OSTYPE" =~ darwin ]]; then
# Mac shit
echo "WE GOT HERE!"
libs="-lraylib"
includes="-I/opt/homebrew/Cellar/raylib/5.0/include"
output_file=game
else
echo "OS not yet implemented"
fi

cc $cflags src/*.cpp $includes $libs -o $output_file
