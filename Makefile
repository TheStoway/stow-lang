CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
SRC = src/main.c src/lexer.c src/parser.c src/interpreter.c
TARGET = stow
TARGET_WIN = stow.exe
EXAMPLE = examples/demo.stow

.PHONY: all clean run linux windows

all: linux

linux: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

windows: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET_WIN)

run: linux
	./$(TARGET) $(EXAMPLE)

run-win: windows
	./$(TARGET_WIN) $(EXAMPLE)

clean:
	rm -f $(TARGET) $(TARGET_WIN) *.o
