connect4: connect4.o board.o check.o
	gcc -Wall -std=c99 -g connect4.c board.c check.c -o connect4
connect4.o: connect4.c
	gcc -Wall -std=c99 -c connect4.c
board.o: board.c board.h
	gcc -Wall -std=c99 -c board.c
check.o: check.c check.h
	gcc -Wall -std=c99 -c check.c
clean: 
	rm -f connect4.o board.o check.o
	rm -f connect4
	rm -f output*.txt