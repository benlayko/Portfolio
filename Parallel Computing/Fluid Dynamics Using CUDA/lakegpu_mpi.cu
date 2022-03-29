/**
 * FILE: lakegpu_mpi.cu
 * DESCRIPTION: Implements the lake surface calculation
 *   using CUDA code and MPI so that it can be spread
 *   across multiple nodes.
 * AUTHORS:
 *   tmleibe2 Trevor M Leibert
 *   bjlayko Benjamin J Layko
 *   pjhamb  Palash Jhamb
 */
#include <stdlib.h>
#include <stdio.h>
#include <cuda_runtime.h>
#include <time.h>

#include "mpi.h"

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

__global__ void EvolveKernel(double *un, double *uc, double *uo, double *pebbles, int nrows, int ncols, double h, double t, double dt)
{
  // determine gpu thread's index into the arrays
  int x = blockIdx.x * blockDim.x + threadIdx.x;
  int y = blockIdx.y * blockDim.y + threadIdx.y;

  // gate off gpu from illegal memory accesses
  if (x > ncols - 1 || y > nrows - 1)
  {
    return;
  }

  int idx = y * ncols + x;

  // set edges to 0
  if (x == 0 || x == ncols - 1)
  {
    un[idx] = 0.;
  }
  else
  {
    un[idx] = 2 * uc[idx] - uo[idx] + VSQR * (dt * dt) * ((uc[idx - 1] + uc[idx + 1] + uc[idx + ncols] + uc[idx - ncols] + ((uc[idx - ncols - 1] + uc[idx - ncols + 1] + uc[idx + ncols - 1] + uc[idx + ncols + 1]) / 4) - 5 * uc[idx]) / (h * h) + f(pebbles[idx], t));
  }
}

static int tpdt(double *t, double dt, double tf)
{
  if ((*t) + dt > tf)
    return 0;
  (*t) = (*t) + dt;
  return 1;
}

void run_gpu(double *u, double *u0, double *u1, double *pebbles, int nrows, int ncols, double h, double end_time, int nthreads)
{
  cudaEvent_t kstart, kstop;
  float ktime;

  /* HW2: Define your local variables here */

  /* Determine rank and number of processes */
  int rank, numproc;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numproc);

  // determine our partners for send/receive
  int back_partner = (rank - 1) < 0 ? numproc - 1 : rank - 1;
  int forward_partner = (rank + 1) >= numproc ? 0 : rank + 1;

  /* device arrays and timing vars - need to add host arrays for mpi */
  double *uc, *uo, *un_d, *uc_d, *uo_d, *pebbles_d, t, dt;

  t = 0.;
  dt = h / 2.;
  /* number of blocks to launch with */
  int nblocks = ncols / nthreads;

  dim3 dimblocks(nblocks, nblocks);
  dim3 dimgrid(nthreads, nthreads);

  /* Set up device timers */
  CUDA_CALL(cudaSetDevice(0));
  CUDA_CALL(cudaEventCreate(&kstart));
  CUDA_CALL(cudaEventCreate(&kstop));

  /* HW2: Add CUDA kernel call preperation code here */
  /* allocate device arrays */
  CUDA_CALL(cudaMalloc(&un_d, sizeof(double) * nrows * ncols));
  CUDA_CALL(cudaMalloc(&uc_d, sizeof(double) * (nrows + 2) * ncols));
  CUDA_CALL(cudaMalloc(&uo_d, sizeof(double) * (nrows + 2) * ncols));
  CUDA_CALL(cudaMalloc(&pebbles_d, sizeof(double) * nrows * ncols));

  /* allocate host arrays for mpi */
  uc = (double *)malloc(sizeof(double) * (nrows + 2) * ncols);
  uo = (double *)malloc(sizeof(double) * (nrows + 2) * ncols);
  if (uc == NULL || uo == NULL)
  {
    fprintf(stderr, "Error allocating host mpi arrays\n");
    exit(1);
  }

  /* copy in initial state into host arrays */
  memcpy(uo + ncols, u0 + ncols, sizeof(double) * nrows * ncols);
  memcpy(uc + ncols, u1 + ncols, sizeof(double) * nrows * ncols);

  /* Start GPU computation timer */
  CUDA_CALL(cudaEventRecord(kstart, 0));

  /* copy in initial state into device arrays */
  CUDA_CALL(cudaMemcpy(pebbles_d, pebbles, sizeof(double) * nrows * ncols, cudaMemcpyHostToDevice));

  /* HW2: Add main lake simulation loop here */

  while (1)
  {
    // synchronize with mpi before running evolve
    // array to hold send/receive requests for both arrays
    MPI_Request all_requests[8];
    // queue up non-blocking receives
    MPI_Irecv(uo, ncols, MPI_DOUBLE, back_partner, 0, MPI_COMM_WORLD, all_requests);
    MPI_Irecv(uc, ncols, MPI_DOUBLE, back_partner, 1, MPI_COMM_WORLD, all_requests + 1);
    MPI_Irecv(uo + (nrows + 1) * ncols, ncols, MPI_DOUBLE, forward_partner, 2, MPI_COMM_WORLD, all_requests + 2);
    MPI_Irecv(uc + (nrows + 1) * ncols, ncols, MPI_DOUBLE, forward_partner, 3, MPI_COMM_WORLD, all_requests + 3);
    // queue up non-blocking sends
    MPI_Isend(uc + ncols, ncols, MPI_DOUBLE, back_partner, 3, MPI_COMM_WORLD, all_requests + 4);
    MPI_Isend(uo + ncols, ncols, MPI_DOUBLE, back_partner, 2, MPI_COMM_WORLD, all_requests + 5);
    MPI_Isend(uc + nrows * ncols, ncols, MPI_DOUBLE, forward_partner, 1, MPI_COMM_WORLD, all_requests + 6);
    MPI_Isend(uo + nrows * ncols, ncols, MPI_DOUBLE, forward_partner, 0, MPI_COMM_WORLD, all_requests + 7);

    // wait on all requests
    MPI_Waitall(8, all_requests, MPI_STATUS_IGNORE);

    // copy freshly synced mpi arrays into the gpu
    CUDA_CALL(cudaMemcpy(uo_d, uo, sizeof(double) * (nrows + 2) * ncols, cudaMemcpyHostToDevice));
    CUDA_CALL(cudaMemcpy(uc_d, uc, sizeof(double) * (nrows + 2) * ncols, cudaMemcpyHostToDevice));

    EvolveKernel<<<dimblocks, dimgrid>>>(un_d, uc_d + ncols, uo_d + ncols, pebbles_d, nrows, ncols, h, t, dt);

    // copy updated arrays back to the mpi holding buffers to be synced - updating orders
    // at the same time

    // offset here since both source and dest have extra rows
    CUDA_CALL(cudaMemcpy(uo + ncols, uc_d + ncols, sizeof(double) * nrows * ncols, cudaMemcpyDeviceToHost));
    // uc doesn't have extra rows, so no offset here
    CUDA_CALL(cudaMemcpy(uc + ncols, un_d, sizeof(double) * nrows * ncols, cudaMemcpyDeviceToHost));

    if (!tpdt(&t, dt, end_time))
      break;
  }

  /* copy out un device array after last timestep */
  CUDA_CALL(cudaMemcpy(u, un_d, sizeof(double) * nrows * ncols, cudaMemcpyDeviceToHost));

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
