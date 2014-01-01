#include <OpenCL/opencl.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpgpu.h"
#include "naive.h"

#define CONCURRENCY 384

#ifdef CL_GPU
#define DEVICE_TYPE CL_DEVICE_TYPE_GPU
#define BITCODE_PATH "src/kernel.gpu32.bc"
#else
#define DEVICE_TYPE CL_DEVICE_TYPE_CPU
#define BITCODE_PATH "src/kernel.cpu64.bc"
#endif

#define DEVICE_INDEX 0


// The various OpenCL objects needed to execute our CL program against a
// given compute device in our system.
cl_device_id     device;
cl_context       context;
cl_command_queue queue;
cl_program       program;
cl_kernel        kernel;
cl_bool          little_endian;

cl_uint mask;

static int create_program_from_bitcode(){
  cl_int err;
  char *path = BITCODE_PATH;

  program = clCreateProgramWithBinary(context, 1, &device, strlen(path),
                                      &path, NULL, &err);
  if(err){
      printf("clCreateProgramWithBinary Error: %d\n", err);
      return err;
  }

  err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  if(err){
      printf("clBuildProgram Error: %d\n", err);
      return err;
  }

  kernel = clCreateKernel(program, "shaforce", &err);
  if(err){
      printf("clCreateKernel Error: %d\n", err);
      return err;
  }

  puts("Loaded program from bitcode");
  return err;
}

static int init_opencl() {
  cl_int err;
  cl_uint num_devices;

  // How many devices of the type requested are in the system?
  err = clGetDeviceIDs(NULL, DEVICE_TYPE, 0, NULL, &num_devices);
  if(err){
      printf("clGetDeviceIDs Error: %d\n", err);
      return err;
  }

  printf("Found %d OpenCL devices\n", num_devices);

  if (DEVICE_INDEX < 0 || DEVICE_INDEX > num_devices - 1) {
    printf("Requsted index (%d) is out of range.\n", DEVICE_INDEX);
    return 1;
  }

  // Grab the requested device.
  cl_device_id all_devices[num_devices];
  err = clGetDeviceIDs(NULL, DEVICE_TYPE, num_devices, all_devices, NULL);
  if(err){
      printf("clGetDeviceIDs (2) Error: %d\n", err);
      return err;
  }

  device = all_devices[DEVICE_INDEX];

  // Dump the device.
  char name[128];
  err = clGetDeviceInfo(device, CL_DEVICE_NAME, 128*sizeof(char), name, NULL);
  if(err){
      printf("clGetDeviceInfo (CL_DEVICE_NAME) Error: %d\n", err);
      return err;
  }

  err = clGetDeviceInfo(device, CL_DEVICE_ENDIAN_LITTLE, sizeof(cl_bool), &little_endian, NULL);
  if(err){
      printf("clGetDeviceInfo (CL_DEVICE_ENDIAN_LITTLE) Error: %d\n", err);
      return err;
  }

  // Create an OpenCL context using this compute device.
  context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  if(err){
      printf("clCreateContext Error: %d\n", err);
      return err;
  }

  // Create a command queue on this device, since we want to use if for
  // running our CL program.
  queue = clCreateCommandQueue(context, device, 0, &err);
  if(err){
      printf("clCreateCommandQueue Error: %d\n", err);
      return err;
  }

  fprintf(stdout, "Initialized OpenCL device: %s\n", name);

  return err;
}

int init_hasher(unsigned char difficulty){
    // Backwards because we're little endian
    mask = 0xffffffff << (32 - difficulty * 4);

    return init_opencl() | create_program_from_bitcode();
}

void* force_hash(hash_args *args){
    int i;
    cl_int err = 0;
    cl_uint words[16], idx = 0, result = 0;
    SHA_CTX msg_ctx;
    size_t concurrency = CONCURRENCY;
    char *block_ptr = &args->msg[BUFFER_LENGTH-BLOCK_LENGTH];

    SHA1_Init(&msg_ctx);
    SHA1_Update(&msg_ctx, args->msg, BUFFER_LENGTH-BLOCK_LENGTH);

    for(i=0; i < 16; i++){
        words[i] = __builtin_bswap32(((cl_uint*)block_ptr)[i]);
    }

    cl_mem block_mem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                      sizeof(cl_uint)*16, words, &err);
    cl_mem result_mem = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR,
                                   sizeof(cl_uint), &result, &err);
    cl_mem mask_mem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                     sizeof(cl_uint), &mask, &err);

    if(err || block_mem == NULL || result_mem == NULL){
        printf("Unable to create OpenCL buffer memory objects: %d\n", err);
        return NULL;
    }

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &block_mem);
    if(err){
        printf("Error setting kernel argument 0: %d\n", err);
        return NULL;
    }

    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &result_mem);
    if(err){
        printf("Error setting kernel argument 1: %d\n", err);
        return NULL;
    }

    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &mask_mem);
    if(err){
        printf("Error setting kernel argument 2: %d\n", err);
        return NULL;
    }

    SHA_LONG *state_ptr = &msg_ctx.h0;
    for(i=3; i < 8; i++){
        err = clSetKernelArg(kernel, i, sizeof(SHA_LONG), state_ptr);
        if(err){
            printf("Error setting kernel argument %d: %d\n", i, err);
            return NULL;
        }

        state_ptr++;
    }

    while(1){
        err = clEnqueueWriteBuffer(queue, block_mem, CL_TRUE, 44, sizeof(cl_uint), &idx,
                             0, NULL, NULL);
        if(err){
            printf("clEnqueueWriteBuffer Error: %d\n", err);
            return NULL;
        }

        err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &concurrency, NULL, 0,
                                     NULL, NULL);
        if(err){
            printf("clEnqueueNDRangeKernel Error: %d\n", err);
            return NULL;
        }

        err = clEnqueueReadBuffer(queue, result_mem, CL_TRUE, 0, sizeof(cl_uint),
                                  &result, 0, NULL, NULL);
        if(err){
            printf("clEnqueueReadBuffer Error: %d\n", err);
            return NULL;
        }

        if(result){
            printf("Found %d %d\n", result-1, idx);
            args->found = 1;
            ((cl_uint*)block_ptr)[12] = result-1;
            ((cl_uint*)block_ptr)[11] = idx;
            break;
        }

        idx++;
    }

    return NULL;
}

int free_hasher(){
    int err = 0;

    // Free up all the CL objects we've allocated.
    err |= clReleaseKernel(kernel);
    err |= clReleaseProgram(program);
    err |= clReleaseCommandQueue(queue);
    err |= clReleaseContext(context);

    if(err){
        puts("Error freeing CL objects");
    }

    return err;
}
