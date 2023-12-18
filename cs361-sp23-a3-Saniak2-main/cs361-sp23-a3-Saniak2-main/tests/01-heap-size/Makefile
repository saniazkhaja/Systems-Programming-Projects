CFLAGS = -Wno-unused-result -std=c17 -g -O0
SRCS=$(wildcard *.c)

OBJS=$(SRCS:.c=.o)

all: a3.o $(OBJS)

a3.o: ../../a3.c
	make -C ../..
	cp ../../a3.o .

%.o : %.c a3.o
	gcc $(CFLAGS) $< -c -o $@
	gcc $(CFLAGS) $@ a3.o -o $(basename $@)

clean:
	rm -rf $(OBJS) a3.o $(basename $(OBJS))
