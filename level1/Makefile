# #
# The OpenCL compiler, used to compile OpenCL kernels.
# #
CLC = /System/Library/Frameworks/OpenCL.framework/Libraries/openclc
FRAMEWORKS = -framework OpenCL

NVCC = nvcc
NVCCFLAGS = -gencode arch=compute_30,code=\"sm_30,compute_30\" -Xptxas="-v"

CC=gcc
CFLAGS = -Wall -O3 -mtune=native -DIMPL_CUDA -g
INCLUDES = -I../libgit2/include -Isrc/ -I/usr/local/cuda/include
LIBS = -L/usr/local/cuda/lib -lgit2 -lcrypto -lssl -lcuda -lcudart

SRCS = src/cuda.c src/common.c
CL_SRCS = src/kernel.cl
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
	$(CC) $(CFLAGS) $(INCLUDES) $(FRAMEWORKS) $(LFLAGS) $(LIBS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

%.o: %.cu
	$(NVCC) $(NVCCFLAGS) -c $< -o $@

# #
# The OpenCL C compilation commands for 32/64bit CPUs and 32bit GPUs:
#
# As an example, to compile for a 32bit GPU:
# openclc -x cl -triple gpu_32-applecl-darwin -emit-llvm-bc kernel.cl -o kernel.bc
# #
%.cpu64.bc: %.cl
	$(CLC) -x cl -triple x86_64-applecl-darwin -emit-llvm-bc $< -o $@

%.gpu32.bc: %.cl
	$(CLC) -x cl -triple gpu_32-applecl-darwin -emit-llvm-bc $< -o $@



clean:
	$(RM) src/*.o src/*.bc *~ build/*

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
