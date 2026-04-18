CC = gcc
CFLAGS = -Wall -I./src
SRC = src/lexer.c src/main.c
OBJ = $(SRC:.c=.o)
TARGET = exemplo.exe

all: $(TARGET)

# Adicionamos -lregex ao final da linha de linkagem
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) -lregex

# Regra para compilar os .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# No MinGW/Windows usamos 'del' em vez de 'rm'
clean:
	if exist src\*.o del /q src\*.o
	if exist $(TARGET) del /q $(TARGET)

run: all
	.\$(TARGET) tests/exemplo.fluent