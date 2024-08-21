eval cc src/level.c -DLEVEL_MAIN $(pkg-config --libs --cflags raylib) -o level_maker
