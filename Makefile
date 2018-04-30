OBJ=main.o ramfs.o hash/hash.o hash/primes.o
CFLAGS=-Wall -g -I ./hash
BIN=fusetest

all: $(BIN)

clean:
	rm $(BIN) $(OBJ)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^
