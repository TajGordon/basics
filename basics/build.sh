eval cc src/*.c $(pkg-config --libs --cflags raylib) -o game
# eval cc yourgame.c -framework IOKit -framework Cocoa -framework OpenGL $(pkg-config --libs --cflags raylib) -o YourGame
