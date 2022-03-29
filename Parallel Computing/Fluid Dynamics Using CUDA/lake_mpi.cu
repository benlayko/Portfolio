/**
 * FILE: lake.cu
 * DESCRIPTION: Original code for the lake calculation with the addition
 *   of the 9pt calculation. It also uses the new CUDA gpu call to calculate
 *   the change as well. In addition it uses MPI to spread the calcuation across
 *   multiple nodes.
 * AUTHORS:
 *   tmleibe2 Trevor M Leibert
 *   bjlayko Benjamin J Layko
 *   pjhamb  Palash Jhamb
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#include "mpi.h"

#define _USE_MATH_DEFINES

#define XMIN 0.0
#define XMAX 1.0
#define YMIN 0.0
#define YMAX 1.0

#define MAX_PSZ 10
#define TSCALE 1.0
#define VSQR 0.1
#define ROOT 0
#define FILENAME_LEN 50
#define DEBUG(message) fprintf(stderr, message " proc %d, line %d file %s\n", rank, __LINE__, __FILE__)

void init(double *u, double *pebbles, int nrows, int ncols);
void evolve(double *un, double *uc, double *uo, double *pebbles, int nrows, int ncols, double h, double dt, double t);
void evolve9pt(double *un, double *uc, double *uo, double *pebbles, int nrows, int ncols, double h, double dt, double t);
int tpdt(double *t, double dt, double end_time);
void print_heatmap(const char *filename, double *u, int nrows, int ncols, double h, int y_offset);
void init_pebbles(double *p, int pn, int n);

void run_cpu(double *u, double *u0, double *u1, double *pebbles, int nrows, int ncols, double h, double end_time);

extern void run_gpu(double *u, double *u0, double *u1, double *pebbles, int nrows, int ncols, double h, double end_time, int nthreads);

int main(int argc, char *argv[])
{

  if (argc != 5)
  {
    printf("Usage: %s npoints npebs time_finish nthreads \n", argv[0]);
    return 1;
  }

  /* MPI variables and init */
  int rank, numproc;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numproc);

  int npoints = atoi(argv[1]);

  // process-specific dimension information
  int ncols = npoints;
  // rows are split evenly across processes, with an extra row on the top and bottom for values
  // to be received from other processes
  int nrows = npoints / numproc;
  // allocate remainder of the split across first n processes
  if (rank < npoints % numproc)
  {
    nrows += 1;
  }

  // set up array of how rows each thread is responsible for
  // and how many total entries a thread is responsible for
  // and displacements into the source array for mpi_gatherv
  int *row_counts = (int *)malloc(sizeof(int) * numproc);
  int *total_counts = (int *)malloc(sizeof(int) * numproc);
  int *displacements = (int *)malloc(sizeof(int) * numproc);
  int running_sum = 0;
  for (int i = 0; i < numproc; i++)
  {
    row_counts[i] = npoints / numproc;
    if (i < npoints % numproc)
    {
      row_counts[i] += 1;
    }
    total_counts[i] = row_counts[i] * ncols;
    displacements[i] = running_sum;
    running_sum += total_counts[i];
  }

  int npebs = atoi(argv[2]);
  double end_time = (double)atof(argv[3]);
  int nthreads = atoi(argv[4]);

  // square area of the total grid
  int narea = npoints * npoints;

  double *u_i0, *u_i1;
  double *u_cpu, *u_gpu, *pebs_local;
  double h;

  double elapsed_cpu, elapsed_gpu;
  struct timeval cpu_start, cpu_end, gpu_start, gpu_end;

  u_i0 = (double *)malloc(sizeof(double) * (nrows + 2) * ncols);
  u_i1 = (double *)malloc(sizeof(double) * (nrows + 2) * ncols);

  // all other arrays use the new smaller grid sizes, but pebbles need to
  // be thrown over the entire original array size
  // we'll be copying the pebbles over to each smaller array, called pebs_local
  pebs_local = (double *)malloc(sizeof(double) * ncols * nrows);

  u_cpu = (double *)malloc(sizeof(double) * nrows * ncols);
  u_gpu = (double *)malloc(sizeof(double) * nrows * ncols);

  printf("Running %s with (%d x %d) grid, until %f, with %d threads\n", argv[0], nrows, ncols, end_time, nthreads);

  h = (XMAX - XMIN) / npoints;

  /* only initialzie pebbles in the root so we all share the same info */
  double *pebs = NULL;
  if (rank == ROOT)
  {
    pebs = (double *)malloc(sizeof(double) * narea);
    if (pebs == NULL)
    {
      fprintf(stderr, "Couldn't allocate pebbles\n");
      exit(1);
    }
    init_pebbles(pebs, npebs, npoints);
  }

  MPI_Scatterv(pebs, total_counts, displacements, MPI_DOUBLE, pebs_local, nrows * ncols, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);
  if (rank == ROOT)
  {
    free(pebs);
  }

  init(u_i0 + ncols, pebs_local, nrows, ncols);
  init(u_i1 + ncols, pebs_local, nrows, ncols);

  // print one file per node
  char filename[FILENAME_LEN + 1];
  snprintf(filename, FILENAME_LEN, "lake_i_%d.dat", rank);
  print_heatmap(filename, u_i0 + ncols, nrows, ncols, h, displacements[rank] / ncols);

  gettimeofday(&cpu_start, NULL);
  run_cpu(u_cpu, u_i0, u_i1, pebs_local, nrows, ncols, h, end_time);
  gettimeofday(&cpu_end, NULL);

  elapsed_cpu = ((cpu_end.tv_sec + cpu_end.tv_usec * 1e-6) - (cpu_start.tv_sec + cpu_start.tv_usec * 1e-6));
  printf("Node %d: CPU took %f seconds\n", rank, elapsed_cpu);

  gettimeofday(&gpu_start, NULL);
  // TODO uncomment this
  run_gpu(u_gpu, u_i0, u_i1, pebs_local, nrows, ncols, h, end_time, nthreads);
  gettimeofday(&gpu_end, NULL);
  elapsed_gpu = ((gpu_end.tv_sec + gpu_end.tv_usec * 1e-6) - (gpu_start.tv_sec + gpu_start.tv_usec * 1e-6));
  printf("Node %d: GPU took %f seconds\n", rank, elapsed_gpu);

  // generate file name by node id
  // TODO change filenames
  snprintf(filename, FILENAME_LEN, "lake_f_cpu_%d.dat", rank);
  print_heatmap(filename, u_cpu, nrows, ncols, h, displacements[rank] / ncols);
  snprintf(filename, FILENAME_LEN, "lake_f_gpu_%d.dat", rank);
  print_heatmap(filename, u_gpu, nrows, ncols, h, displacements[rank] / ncols);

  free(u_i0);
  free(u_i1);
  free(pebs_local);
  free(u_cpu);
  free(u_gpu);
  free(row_counts);
  free(total_counts);
  free(displacements);

  // teardown mpi
  MPI_Finalize();

  return 0;
}

void run_cpu(double *u, double *u0, double *u1, double *pebbles, int nrows, int ncols, double h, double end_time)
{
  int rank, numproc;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numproc);

  // determine our partners for send/receive
  int back_partner = (rank - 1) < 0 ? numproc - 1 : rank - 1;
  int forward_partner = (rank + 1) >= numproc ? 0 : rank + 1;

  double *un, *uc, *uo;
  double t, dt;

  un = (double *)malloc(sizeof(double) * nrows * ncols);
  // need 2 extra rows for the arrays that will be communicated back and forth
  uc = (double *)malloc(sizeof(double) * (nrows + 2) * ncols);
  uo = (double *)malloc(sizeof(double) * (nrows + 2) * ncols);
  if (un == NULL || uc == NULL || uo == NULL)
  {
    fprintf(stderr, "Error allocating run arrays\n");
    exit(1);
  }

  // copy initialized parts of history matrices into their local counterparts
  memcpy(uo + ncols, u0 + ncols, sizeof(double) * nrows * ncols);
  memcpy(uc + ncols, u1 + ncols, sizeof(double) * nrows * ncols);

  t = 0.;
  dt = h / 2.;

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

    // evolve the simulation
    // offset into the "real" data inside of uc and uo
    // first and last rows are the "extra" ones
    evolve9pt(un, uc + ncols, uo + ncols, pebbles, nrows, ncols, h, dt, t);

    memcpy(uo + ncols, uc + ncols, sizeof(double) * nrows * ncols);
    memcpy(uc + ncols, un, sizeof(double) * nrows * ncols);

    if (!tpdt(&t, dt, end_time))
      break;
  }

  memcpy(u, un, sizeof(double) * nrows * ncols);
}

void init_pebbles(double *p, int pn, int n)
{
  int i, j, k, idx;
  int sz;

  srand(time(NULL));
  memset(p, 0, sizeof(double) * n * n);

  for (k = 0; k < pn; k++)
  {
    i = rand() % (n - 4) + 2;
    j = rand() % (n - 4) + 2;
    sz = rand() % MAX_PSZ;
    // transpose placements
    idx = i + j * n;
    p[idx] = (double)sz;
  }
}

double f(double p, double t)
{
  return -expf(-TSCALE * t) * p;
}

int tpdt(double *t, double dt, double tf)
{
  if ((*t) + dt > tf)
    return 0;
  (*t) = (*t) + dt;
  return 1;
}

void init(double *u, double *pebbles, int nrows, int ncols)
{
  int i, j, idx;

  for (i = 0; i < nrows; i++)
  {
    for (j = 0; j < ncols; j++)
    {
      idx = j + i * ncols;
      u[idx] = f(pebbles[idx], 0.0);
    }
  }
}

void evolve(double *un, double *uc, double *uo, double *pebbles, int nrows, int ncols, double h, double dt, double t)
{
  int i, j, idx;

  for (i = 0; i < nrows; i++)
  {
    for (j = 0; j < ncols; j++)
    {
      idx = j + i * ncols;

      if (j == 0 || j == ncols - 1)
      {
        un[idx] = 0.;
      }
      else
      {
        un[idx] = 2 * uc[idx] - uo[idx] + VSQR * (dt * dt) * ((uc[idx - 1] + uc[idx + 1] + uc[idx + ncols] + uc[idx - ncols] - 4 * uc[idx]) / (h * h) + f(pebbles[idx], t));
      }
    }
  }
}

/**
 * @brief Version of evolve usinga 9-point stencil to calculate the difference
 *
 * @param un m by n results matrix
 * @param uc m + 2 by n input history matrix
 * @param uo m + 2 by n input history matrix
 * @param pebbles m by n input pebble matrix
 * @param nrows number of rows
 * @param ncols number of cols
 * @param h height of the overall matrix
 * @param dt time step
 * @param t current time
 */
void evolve9pt(double *un, double *uc, double *uo, double *pebbles, int nrows, int ncols, double h, double dt, double t)
{
  int i, j, idx;
  double sum = 0.0;
  for (i = 0; i < nrows; i++)
  {
    for (j = 0; j < ncols; j++)
    {
      idx = j + i * ncols;

      if (j == 0 || j == ncols - 1)
      {
        un[idx] = 0.;
      }
      else
      {
        // 9 point pseudocode adapted to C
        un[idx] = 2 * uc[idx] - uo[idx] + VSQR * (dt * dt) * ((uc[idx - 1] + uc[idx + 1] + uc[idx + ncols] + uc[idx - ncols] + ((uc[idx - ncols - 1] + uc[idx - ncols + 1] + uc[idx + ncols - 1] + uc[idx + ncols + 1]) / 4) - 5 * uc[idx]) / (h * h) + f(pebbles[idx], t));
      }
      sum += un[idx];
    }
  }
}

void print_heatmap(const char *filename, double *u, int nrows, int ncols, double h, int y_offset)
{
  int i, j, idx;

  FILE *fp = fopen(filename, "w");

  for (i = 0; i < nrows; i++)
  {
    for (j = 0; j < ncols; j++)
    {
      idx = j + i * ncols;
      // transpose print cols
      fprintf(fp, "%f %f %f\n", j * h, (i + y_offset) * h, u[idx]);
    }
  }

  fclose(fp);
}
