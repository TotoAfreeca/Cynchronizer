OBJ = main.o functions.o helpers.o

all: Cynchronizer
Cynchronizer: $(OBJ)
	gcc $(OBJ) -o Cynchronizer
$(OBJ): functions.h helpers.h

.PHONY: clean
clean:
	rm -rf *.o
test: 	
	./test.sh


