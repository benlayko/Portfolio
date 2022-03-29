/**
 * FILE: lakegpu.cu
 * DESCRIPTION: Implements the lake surface calculation
 *   using CUDA code.
 * AUTHORS:
 *   tmleibe2 Trevor M Leibert
 *   bjlayko Benjamin J Layko
 *   pjhamb  Palash Jhamb
 */
#include <stdlib.h>
#include <stdio.h>
#include <cuda_runtime.h>
#include <time.h>

#define __DEBUG

#define XMIN 0.0
#define XMAX 1.0
#define YMIN 0.0
#define YMAX 1.0

#define MAX_PSZ 10
#define TSCALE 1.0
#define VSQR 0.1

#define CUDA_CALL(err) __cudaSafeCall(err, __FILE__, __LINE__)
#define CUDA_CHK_ERR() __cudaCheckError(__FILE__, __LINE__)

/**************************************
 * void __cudaSafeCall(cudaError err, const char *file, const int line)
 * void __cudaCheckError(const char *file, const int line)
 *
 * These routines were taken from the GPU Computing SDK
 * (http://developer.nvidia.com/gpu-computing-sdk) include file "cutil.h"
 **************************************/
inline void __cudaSafeCall(cudaError err, const char *file, const int line)
{
#ifdef __DEBUG

#pragma warning(push)
#pragma warning(disable : 4127) // Prevent warning on do-while(0);
  do
  {
    if (cudaSuccess != err)
    {
      fprintf(stderr, "cudaSafeCall() failed at %s:%i : %s\n",
              file, line, cudaGetErrorString(err));
      exit(-1);
    }
  } while (0);
#pragma warning(pop)
#endif // __DEBUG
  return;
}

inline void __cudaCheckError(const char *file, const int line)
{
#ifdef __DEBUG
#pragma warning(push)
#pragma warning(disable : 4127) // Prevent warning on do-while(0);
  do
  {
    cudaError_t err = cudaGetLastError();
    if (cudaSuccess != err)
    {
      fprintf(stderr, "cudaCheckError() failed at %s:%i : %s.\n",
              file, line, cudaGetErrorString(err));
      exit(-1);
    }
    // More careful checking. However, this will affect performance.
    // Comment if not needed.
    /*err = cudaThreadSynchronize();
    if( cudaSuccess != err )
    {
      fprintf( stderr, "cudaCheckError() with sync failed at %s:%i : %s.\n",
               file, line, cudaGetErrorString( err ) );
      exit( -1 );
    }*/
  } while (0);
#pragma warning(pop)
#endif // __DEBUG
  return;
}

static __device__ double f(double p, double t)
{
  return -expf(-TSCALE * t) * p;
}

__global__ void EvolveKernel(double *un, double *uc, double *uo, double *pebbles, int n, double h, double t, double dt)
{
  // determine gpu thread's index into the arrays
  int x = blockIdx.x * blockDim.x + threadIdx.x;
  int y = blockIdx.y * blockDim.y + threadIdx.y;

  if (x > n - 1 || y > n - 1)
  {
    return;
  }
  int idx = y * n + x;

  // set edges to 0
  if (x == 0 || x == n - 1 || y == 0 || y == n - 1)
  {
    un[idx] = 0.;
  }
  else
  {
    un[idx] = 2 * uc[idx] - uo[idx] + VSQR * (dt * dt) * ((uc[idx - 1] + uc[idx + 1] + uc[idx + n] + uc[idx - n] + ((uc[idx - n - 1] + uc[idx - n + 1] + uc[idx + n - 1] + uc[idx + n + 1]) / 4) - 5 * uc[idx]) / (h * h) + f(pebbles[idx], t));
  }
}

static int tpdt(double *t, double dt, double tf)
{
  if ((*t) + dt > tf)
    return 0;
  (*t) = (*t) + dt;
  return 1;
}

void run_gpu(double *u, double *u0, double *u1, double *pebbles, int n, double h, double end_time, int nthreads)
{
  cudaEvent_t kstart, kstop;
  float ktime;

  /* HW2: Define your local variables here */

  /* device arrays and timing vars*/
  double *un_d, *uc_d, *uo_d, *pebbles_d, t, dt;

  t = 0.;
  dt = h / 2.;
  /* number of blocks to launch with */
  int blocks = n / nthreads;

  dim3 dimblocks(blocks, blocks);
  dim3 dimgrid(nthreads, nthreads);

  /* Set up device timers */
  CUDA_CALL(cudaSetDevice(0));
  CUDA_CALL(cudaEventCreate(&kstart));
  CUDA_CALL(cudaEventCreate(&kstop));

  /* HW2: Add CUDA kernel call preperation code here */
  /* allocate device arrays */
  CUDA_CALL(cudaMalloc(&un_d, sizeof(double) * n * n));
  CUDA_CALL(cudaMalloc(&uc_d, sizeof(double) * n * n));
  CUDA_CALL(cudaMalloc(&uo_d, sizeof(double) * n * n));
  CUDA_CALL(cudaMalloc(&pebbles_d, sizeof(double) * n * n));

  /* Start GPU computation timer */
  CUDA_CALL(cudaEventRecord(kstart, 0));

  /* copy in initial state into device arrays */
  CUDA_CALL(cudaMemcpy(uo_d, u0, sizeof(double) * n * n, cudaMemcpyHostToDevice));
  CUDA_CALL(cudaMemcpy(uc_d, u1, sizeof(double) * n * n, cudaMemcpyHostToDevice));
  CUDA_CALL(cudaMemcpy(pebbles_d, pebbles, sizeof(double) * n * n, cudaMemcpyHostToDevice));

  /* HW2: Add main lake simulation loop here */

  while (1)
  {
    EvolveKernel<<<dimblocks, dimgrid>>>(un_d, uc_d, uo_d, pebbles_d, n, h, t, dt);

    CUDA_CALL(cudaMemcpy(uo_d, uc_d, sizeof(double) * n * n, cudaMemcpyDeviceToDevice));
    CUDA_CALL(cudaMemcpy(uc_d, un_d, sizeof(double) * n * n, cudaMemcpyDeviceToDevice));

    if (!tpdt(&t, dt, end_time))
      break;
  }

  /* copy out un device array after last timestep */
  CUDA_CALL(cudaMemcpy(u, un_d, sizeof(double) * n * n, cudaMemcpyDeviceToHost));

  /* Stop GPU computation timer */
  CUDA_CALL(cudaEventRecord(kstop, 0));
  CUDA_CALL(cudaEventSynchronize(kstop));
  CUDA_CALL(cudaEventElapsedTime(&ktime, kstart, kstop));
  printf("GPU computation: %f msec\n", ktime);

  /* HW2: Add post CUDA kernel call processing and cleanup here */
  CUDA_CALL(cudaFree(un_d));
  CUDA_CALL(cudaFree(uc_d));
  CUDA_CALL(cudaFree(uo_d));
  CUDA_CALL(cudaFree(pebbles_d));

  /* timer cleanup */
  CUDA_CALL(cudaEventDestroy(kstart));
  CUDA_CALL(cudaEventDestroy(kstop));
}
