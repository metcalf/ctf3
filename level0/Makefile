CC=gcc
CFLAGS = -Wall -O3 -march=native -DINFILE='"data/words"'
INCLUDES = -Isrc/

.PHONY: depend clean

all: build/bloom level0

build/bloom: src/bloom.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

level0: src/filter.o src/level0.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

src/filter.bin: build/bloom
	build/bloom

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.bin
	objcopy -B i386 -I binary -O elf64-x86-64 $< $@

clean:
	$(RM) src/*.o src/*.bin *~ build/*

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
