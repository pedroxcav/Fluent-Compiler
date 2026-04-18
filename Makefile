CC = gcc
FLAGS = -Wall
SRC = src/lexer.c src/main.c
OBJ = build/lexer.o build/main.o
EXE = build/exemplo.exe
INCLUDE = -I./src

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(FLAGS) $(INCLUDE) -o $(EXE) $(OBJ) -lregex

build/lexer.o: src/lexer.c
	$(CC) $(FLAGS) $(INCLUDE) -c src/lexer.c -o build/lexer.o

build/main.o: src/main.c
	$(CC) $(FLAGS) $(INCLUDE) -c src/main.c -o build/main.o

re: clean all
	$(EXE) tests/exemplo.fluent

clean:
	if exist build\*.o del /q build\*.o
	if exist build\*.exe del /q build\*.exe