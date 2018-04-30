OBJ=main.o ramfs.o hash/hash.o hash/primes.o hash/common.o
FUSEOBJ=ramfs.o hash/hash.o hash/primes.o hash/common.o
CFLAGS=-Wall -g -I ./hash
BIN=fusetest
FUSE=fuse_ram

all: $(BIN) $(FUSE)

clean:
	rm $(BIN) $(OBJ) $(FUSE)

fuse_ramfs.o: fuse_ramfs.c
	$(CC) -c $(CFLAGS) -D_FILE_OFFSET_BITS=64 -o $@ $^

$(FUSE): $(FUSEOBJ) fuse_ramfs.o
	$(CC) $(CFLAGS) -o $@ $^ -lfuse

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^
