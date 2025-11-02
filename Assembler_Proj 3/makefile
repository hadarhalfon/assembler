assembler : main.o macros.o helpers.o symbolTable.o order.o decode.o word.o input.o output.o
	gcc -g -ansi -Wall -pedantic main.o macros.o helpers.o symbolTable.o order.o decode.o word.o input.o output.o -o assembler

main.o : main.c macros.h order.h helpers.h output.h
	gcc -c -ansi -Wall -pedantic main.c -o main.o

macros.o: macros.c macros.h
	gcc -c -ansi -Wall -pedantic macros.c -o macros.o

helpers.o: helpers.c helpers.h
	gcc -c -ansi -Wall -pedantic helpers.c -o helpers.o

symbolTable.o : symbolTable.c symbolTable.h
	gcc -c -ansi -Wall -pedantic symbolTable.c -o symbolTable.o

order.o: order.c order.h symbolTable.h word.h
	gcc -c -ansi -Wall -pedantic order.c -o order.o

decode.o: decode.c decode.h
	gcc -c -ansi -Wall -pedantic decode.c -o decode.o

word.o: word.c word.h
	gcc -c -ansi -Wall -pedantic word.c -o word.o

input.o: input.c input.h
	gcc -c -ansi -Wall -pedantic input.c -o input.o

output.o: output.c output.h
	gcc -c -ansi -Wall -pedantic output.c -o output.o


