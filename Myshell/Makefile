all:	myshell

myshell:	myshell.o
	gcc myshell.o -o myshell

myshell.o:	myshell.c
	gcc -c myshell.c -o myshell.o

clean:
	rm -f myshell myshell.o core *~