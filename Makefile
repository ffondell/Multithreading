all: second.c
	gcc -g -Wall -pthread -o second second.c -lm

clean:
	$(RM) second
