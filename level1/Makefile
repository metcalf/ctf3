CC=gcc
CFLAGS = -Wall -O3 -mtune=native
INCLUDES = -I../libgit2/include -Isrc/
LIBS = -lgit2 -lcrypto -lssl
SRCS = src/naive.c src/main.c src/common.c
OBJS = $(SRCS:.c=.o)
MAIN = level1

BENCHMARK_SRCS = src/naive.c src/common.c src/benchmark.c
BENCHMARK_OBJS = $(BENCHMARK_SRCS:.c=.o)
BENCHMARK_MAIN = benchmark

.PHONY: depend clean

all:    $(MAIN)

$(MAIN): $(OBJS) $(BENCHMARK_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BENCHMARK_MAIN) $(BENCHMARK_OBJS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) src/*.o *~ $(MAIN) $(BENCHMARK_MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
