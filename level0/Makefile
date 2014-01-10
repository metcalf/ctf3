CC=g++
CFLAGS = -Wall -O3 -static
INCLUDES = -I../marisa-0.2.4/lib -Isrc/
LIBS = -L../marisa-0.2.4/lib/.libs -lmarisa

SRCS = src/level0.c
DATA = src/trie.bin

OBJS = $(SRCS:.c=.o)
OBJS += $(DATA:.bin=.o)

.PHONY: depend clean

all: build/level0

build/%: $(OBJS) src/%.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.bin
	objcopy -B i386 -I binary -O elf64-x86-64 $< $@

clean:
	$(RM) src/*.o *~ build/*

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
