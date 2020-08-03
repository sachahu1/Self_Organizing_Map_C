CC=gcc -std=c11 
CFLAGS=-W -Wall -pedantic -g -O3 
LDFLAGS=-lm 
EXEC=Iris
SRC= Iris2.c
OBJ= $(SRC:.c=.o)

all: $(EXEC)

Iris: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o Iris

Nloop:
	for i in `seq 10`; do time -p ./Iris config.txt iris_data.txt; done

