# CC=c++
# CFLAGS=-Wall -std=c++20

# TARGET=main
# SRC=src/main.cpp
# LIBS=-lraylib
# INCLUDES=-I/opt/homebrew/Cellar/raylib/5.0/include

# all: $(TARGET)

# # Rule to create the executable
# $(TARGET): $(SRC)
# 	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -o $(TARGET) $(SRC)
# 	echo "Recompiling"

# # Rule to run the executable
# run: $(TARGET)
# 	@./$(TARGET)

# # Rule to clean up the build files
# clean:
# 	rm -f $(TARGET)

# # Specify that 'run' and 'clean' are not files
# .PHONY: all run clean game


all: build

build:
	echo "building"
	@./build.sh

run: build
	echo "Running and building"
	@./game
