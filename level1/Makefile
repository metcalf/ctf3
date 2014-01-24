NVCC = nvcc
NVCCFLAGS = -gencode arch=compute_30,code=\"sm_30,compute_30\" -Xptxas="-v" -O3

CC=gcc
CFLAGS = -Wall -O3 -mtune=native -DIMPL_CUDA
INCLUDES = -I../libgit2/include -Isrc/ -I/usr/local/cuda/include -I/opt/nvidia/cuda/include
LIBS = -L/usr/local/cuda/lib -L../libgit2-development/build -L/opt/nvidia/cuda/lib64 -L/lib64/ -lgit2 -lcrypto -lssl -lcuda -lcudart -lpthread

SRCS = src/cuda.c src/common.c
CU_SRCS = src/sha1.cu

OBJS = $(SRCS:.c=.o)
OBJS += $(CU_SRCS:.cu=.o)

# #
# For each OpenCL C source file, we want to build:
#
#   file.cpu64.bc, and file.gpu32.bc, where file is the
#   source name preceding the .cl extension.
# #
BITCODE += ${CL_SRCS:.cl=.cpu64.bc}
BITCODE += ${CL_SRCS:.cl=.gpu32.bc}

.PHONY: depend clean
.SECONDARY:

all: build/level1 build/benchmark $(BITCODE) $(OBJS)

build/%: $(OBJS) src/%.o
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

%.o: %.cu
	$(NVCC) $(NVCCFLAGS) -c $< -o $@

%.ptx: %.cu
	$(NVCC) $(NVCCFLAGS) -lineinfo --ptx $< -o $@

clean:
	$(RM) src/*.o src/*.bc *~ build/*

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
