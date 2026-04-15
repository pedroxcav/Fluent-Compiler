all:
	gcc src/main/main.c -I src/token/ -o fluent

run: all
	./fluent

clean:
	rm -f fluent