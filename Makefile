CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L
SRC     = src/main.c \
          src/lexer/lexer.c \
          src/ast/ast.c \
          src/parser/parser.c \
          src/semantic/semantic.c
TARGET  = build/fluent.exe

all: build

build:
	if not exist build mkdir build
	$(CC) $(CFLAGS) -Isrc $(SRC) -o $(TARGET) -lregex

clean:
	if exist build rd /s /q build

run:
	$(TARGET) tests/exemplo.fluent