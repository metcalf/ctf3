CC=gcc
CFLAGS = -Wall -O3
INCLUDES = -I../libgit2/include -Isrc/
LIBS = -lgit2 -lcrypto -lssl
SRCS = src/naive.c src/main.c src/common.c
OBJS = $(SRCS:.c=.o)
MAIN = level1

.PHONY: depend clean

all:    $(MAIN)

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) src/*.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
