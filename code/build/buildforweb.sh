# emcc -o index.html ../src/main.c \
# -Os -Wall ~/raylib-5.0_webassembly/lib/libraylib.a \
# -I. -I~/raylib-5.0_webassembly/include/raylib.h \
# -I~/raylib-5.0_webassembly/include/raymath.h -L. \
# -L~/raylib-5.0_webassembly/lib/librarylib.a \
# --shell-file /Users/tajgordon/Developer/raylib/src/minshell.html \
# -s USE_GLFW=3 -s ASYNCIFY -DPLATFORM_WEB --preload-file ../assets  \
# -s ASSERTIONS=1 -s 'EXPORTED_FUNCTIONS=["_free","_malloc","_main"]' \
# -s TOTAL_MEMORY=256MB -s TOTAL_STACK=64MB

#--------------------------------#
emcc -o index.html ../src/main.c -Os -Wall /Users/tajgordon/Developer/raylib/src/libraylib.a -I. -I/Users/tajgordon/Developer/raylib/src/raylib.h -L. -L/Users/tajgordon/Developer/raylib/src/librarylib.a -s USE_GLFW=3 -s ASYNCIFY -DPLATFORM_WEB --preload-file ../assets -s FORCE_FILESYSTEM=1 -s TOTAL_MEMORY=67108864 -s 'EXPORTED_FUNCTIONS=["_free","_malloc","_main"]' # --shell-file /Users/tajgordon/Developer/raylib/src/minshell.html
