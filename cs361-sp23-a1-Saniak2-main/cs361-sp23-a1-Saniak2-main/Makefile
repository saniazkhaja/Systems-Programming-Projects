all: main.o
	# gcc -g main.o -o a1
	gcc main.o lib.o -o a1 

main.o: main.c
	gcc -g main.c -c -o main.o

clean:
	rm -rf main.o a1 d
