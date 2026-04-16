all:
	gcc src/main.c src/lexer/lexer.c -I src/token/ -I src/lexer/ -o fluent

run: all
	./fluent source.fluent

clean:
	rm -f fluent