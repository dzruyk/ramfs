OBJ=main.o ramfs.o vector/vector.o
CFLAGS=-Wall -g
BIN=fusetest

all: $(BIN)

clean:
	rm $(BIN) $(OBJ)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^
