/**
 * FILE: p2_mpi.c
 * DESCRIPTION: Parallel version of p2.c, with options for blocking/non-blocking
 *   communication and MPI_Gather/custom gather implementation.
 * AUTHORS:
 *   tmleibe2 Trevor M Leibert
 *   bjlayko Benjamin J Layko
 *   pjhamb  Palash Jhamb
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "mpi.h"

/* first grid point */
#define XI 1.0
/* last grid point */
#define XF 100.0
/* Root process */
#define ROOT 0

/* function declarations */
double fn(double);
void print_function_data(int, double *, double *, double *);
int main(int, char **);

void blocking_boundary_send(int, int, int, double *);
void non_blocking_boundary_send(int, int, int, double *);
void mpi_gather_v(int, int, int, int, double *, double *, double *, double *);
void custom_blocking_gather(int, int, int, int, double *, double *, double *, double *);
void custom_non_blocking_gather(int, int, int, int, double *, double *, double *, double *);

// Our code runs here
int main(int argc, char *argv[])
{
    // Ensures that the program is given the right number of arguments
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s {number_of_gridpoints} {point-to-point_type} {gather_type}\n", argv[0]);
        exit(1);
    }

    const int NGRID = atoi(argv[1]);
    const bool BLOCKING = atoi(argv[2]) == 0;
    const bool GATHER = atoi(argv[3]) == 0;

    // loop index
    int i;

    // domain array and step size
    double *xc = malloc(sizeof(double) * (NGRID + 2));
    double dx;

    // construct the grid
    for (i = 1; i <= NGRID; i++)
    {
        xc[i] = XI + (XF - XI) * (double)(i - 1) / (double)(NGRID - 1);
    }

    // step size and boundary points
    dx = xc[2] - xc[1];
    xc[0] = xc[1] - dx;
    xc[NGRID + 1] = xc[NGRID] + dx;

    // Initialize MPI and set process variables
    int numproc, rank, len;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double start_time = MPI_Wtime();

    // function array, and derivative
    // the size will be dependent on the
    // number of processors used
    // to the program
    double *yc, *dyc;

    //"real" grid indices
    int imin, imax;

    imin = 1;
    imax = NGRID;

    const int rem = NGRID % numproc;
    int slice_size = NGRID / numproc;
    // share the remainder (n) among the first n processes
    if (rank < rem)
    {
        slice_size++;
    }

    yc = malloc((slice_size + 2) * sizeof(double));
    dyc = malloc((slice_size + 2) * sizeof(double));

    const int base_count = NGRID / numproc;
    const int extra = rank < rem ? rank : rem;
    const int start = base_count * rank + extra;

    // define the function for this slice
    for (i = 1; i <= slice_size; i++)
    {
        yc[i] = fn(xc[start + i]);
    }

    // dispatch to appropriate boundary sending function
    // if we've got more than one process
    if (numproc > 1)
    {
        if (BLOCKING)
        {
            blocking_boundary_send(slice_size, rank, numproc, yc);
        }
        else
        {
            non_blocking_boundary_send(slice_size, rank, numproc, yc);
        }
    }

    // set remaining boundary values
    if (rank == ROOT)
    {
        yc[0] = fn(xc[0]);
    }
    if (rank == numproc - 1)
    {
        yc[slice_size + 1] = fn(xc[NGRID + 1]);
    }

    // compute the derivative using first-order finite differencing
    //
    //   d           f(x + h) - f(x - h)
    //  ---- f(x) ~ --------------------
    //   dx                 2 * dx
    //
    for (i = 1; i <= slice_size; i++)
    {
        dyc[i] = (yc[i + 1] - yc[i - 1]) / (2.0 * dx);
    }

    // allocate receiving buffers for the root process
    // all other processes keep these as NULL
    double *r_y_buf = NULL, *r_dy_buf = NULL;
    if (rank == ROOT)
    {
        r_y_buf = malloc(sizeof(double) * NGRID);
        r_dy_buf = malloc(sizeof(double) * NGRID);

        if (r_y_buf == NULL || r_dy_buf == NULL)
        {
            MPI_Finalize();
            exit(1);
        }
    }

    // dispatch to appropriate gathering function
    if (GATHER)
    {
        mpi_gather_v(rank, numproc, NGRID, slice_size, yc, dyc, r_y_buf, r_dy_buf);
    }
    else if (BLOCKING)
    {
        custom_blocking_gather(rank, numproc, NGRID, slice_size, yc, dyc, r_y_buf, r_dy_buf);
    }
    else
    {
        custom_non_blocking_gather(rank, numproc, NGRID, slice_size, yc, dyc, r_y_buf, r_dy_buf);
    }

    double end_time = MPI_Wtime();
    printf("Process %d took %e seconds.\n", rank, end_time - start_time);

    // only let the root process print
    if (rank == ROOT)
    {
        // don't need to offset since we allocated the exact amount
        // of elements for the receiving buffer
        print_function_data(NGRID, xc + 1, r_y_buf, r_dy_buf);
        // free root's receiving buffers
        free(r_y_buf);
        free(r_dy_buf);
    }

    MPI_Finalize();

    // free allocated memory
    free(yc);
    free(dyc);
    free(xc);

    return EXIT_SUCCESS;
}

/* Boundary functions ********************************************************/

/*This function handles the point to point communication between the different processors
 *in using blocking communication
 */
void blocking_boundary_send(int slice_size, int rank, int numproc, double *yc)
{
    MPI_Status status;
    // Send the information needed to the other processors in the forward direction
    // In this case the root does not need to recieve, and the last element does not need to send
    if (rank == ROOT)
    {
        MPI_Send(yc + slice_size, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
    }
    else if (rank == numproc - 1)
    {
        MPI_Recv(yc, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &status);
    }
    else
    {
        MPI_Send(yc + slice_size, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
        MPI_Recv(yc, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &status);
    }

    // Send the information in the backward direction
    // In this case the root does not need to send, and the last element does not need to recieve
    if (rank == ROOT)
    {
        MPI_Recv(yc + 1 + slice_size, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &status);
    }
    else if (rank == numproc - 1)
    {
        MPI_Send(yc + 1, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Send(yc + 1, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
        MPI_Recv(yc + 1 + slice_size, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &status);
    }
}

/*This function handles the point to point communication between the different processors
 *in using nonblocking communication. Currently the implmentation has the wait and requests created in the
 *function, but this may need to be moved to increase efficency.
 */
void non_blocking_boundary_send(int slice_size, int rank, int numproc, double *yc)
{
    // Create storage for all sends and receives
    // note we don't need to wait for sends since they don't affect our
    // further computations and we won't be modifying any of the relevant
    // parts of yc after this call
    MPI_Request left_send, right_send, left_recv, right_recv;

    // Send the information needed to the other processors in the forward direction
    // In this case the root does not need to recieve, and the last element does not need to send
    if (rank == ROOT)
    {
        MPI_Isend(yc + slice_size, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &right_send);
        // MPI_Request_free(&right_send);
    }
    else if (rank == numproc - 1)
    {
        MPI_Irecv(yc, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &left_recv);
    }
    else
    {
        // Note here we have tweaked the order of send and recieve to better utilize the nonblocking communication.
        MPI_Irecv(yc, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &left_recv);
        MPI_Isend(yc + slice_size, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &right_send);
        // MPI_Request_free(&right_send);
    }

    // Send the information in the backward direction
    // In this case the root does not need to send, and the last element does not need to recieve
    if (rank == ROOT)
    {
        MPI_Irecv(yc + 1 + slice_size, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &right_recv);
    }
    else if (rank == numproc - 1)
    {
        MPI_Isend(yc + 1, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &left_send);
        // MPI_Request_free(&left_send);
    }
    else
    {
        // Note here we have tweaked the order of send and recieve to better utilize the nonblocking communication.
        MPI_Irecv(yc + 1 + slice_size, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &right_recv);
        MPI_Isend(yc + 1, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &left_send);
        // MPI_Request_free(&left_send);
    }

    // wait for recieves to complete
    if (rank == ROOT)
    {
        MPI_Wait(&right_recv, MPI_STATUS_IGNORE);
        MPI_Wait(&right_send, MPI_STATUS_IGNORE);
    }
    else if (rank == numproc - 1)
    {
        MPI_Wait(&left_recv, MPI_STATUS_IGNORE);
        MPI_Wait(&left_send, MPI_STATUS_IGNORE);
    }
    else
    {
        MPI_Wait(&left_recv, MPI_STATUS_IGNORE);
        MPI_Wait(&right_recv, MPI_STATUS_IGNORE);
        MPI_Wait(&left_send, MPI_STATUS_IGNORE);
        MPI_Wait(&right_send, MPI_STATUS_IGNORE);
    }
}
/* END Boundary functions ****************************************************/

/* Gather functions **********************************************************/

/**
 * @brief Gathers function value and derivatives using MPI_Gatherv.
 *
 * @param rank rank of the calling process
 * @param numproc number of total processes
 * @param ngrid total number of datapoints
 * @param slice_size number of elements to add in (size of yc and dyc)
 * @param yc function value input buffer
 * @param dyc function derivative input buffer
 * @param r_y gathered function output buffer (NULL if not root)
 * @param r_dy gathered derivative output buffer (NULL if not root)
 */
void mpi_gather_v(int rank, int numproc, int gridsize, int slice_size, double *yc, double *dyc, double *r_y, double *r_dy)
{
    // prepare counts and offsets if we're in the root process
    int *counts = NULL;
    int *offsets = NULL;
    const int rem = gridsize % numproc;
    const int base_count = gridsize / numproc;
    if (rank == ROOT)
    {
        counts = malloc(sizeof(int) * numproc);
        offsets = malloc(sizeof(int) * numproc);
        if (counts == NULL || offsets == NULL)
        {
            MPI_Finalize();
            exit(1);
        }

        // compute counts and offsets
        int offset = 0;
        for (int i = 0; i < numproc; i++)
        {
            int cur_count = base_count;
            if (i < rem)
            {
                cur_count++;
            }
            counts[i] = cur_count;
            offsets[i] = offset;
            offset += cur_count;
        }
    }

    MPI_Gatherv(yc + 1, slice_size, MPI_DOUBLE, r_y, counts, offsets, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);
    MPI_Gatherv(dyc + 1, slice_size, MPI_DOUBLE, r_dy, counts, offsets, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);

    // free the prepared counts and offsets if we're in the root process
    if (rank == ROOT)
    {
        free(offsets);
        free(counts);
    }
}

// This function uses blocking point to point commmunication to collect all the information at the end.
void custom_blocking_gather(int rank, int numproc, int gridsize, int slice_size, double *yc, double *dyc, double *r_y, double *r_dy)
{
    // The root has p - 1 recieves for each buffer, while the other processors send the root the data
    if (rank == 0)
    {
        // root can copy its own data into the result buffer
        memcpy(r_y, yc + 1, slice_size * sizeof(double));
        memcpy(r_dy, dyc + 1, slice_size * sizeof(double));

        // have to track the "extra" datapoints that need to be recieved
        // due to uneven sharing of data
        const int rem = gridsize % numproc;
        const int base_count = gridsize / numproc;
        int extra = rem > 0 ? 1 : 0;
        for (int i = 1; i < numproc; i++)
        {
            MPI_Recv(r_y + (i * base_count) + extra, slice_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(r_dy + (i * base_count) + extra, slice_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            extra += rem > i ? 1 : 0;
        }
    }
    else
    {
        MPI_Send(yc + 1, slice_size, MPI_DOUBLE, ROOT, 0, MPI_COMM_WORLD);
        MPI_Send(dyc + 1, slice_size, MPI_DOUBLE, ROOT, 0, MPI_COMM_WORLD);
    }
}

// This function uses non-blocking point to point commmunication to collect all the information at the end.
void custom_non_blocking_gather(int rank, int numproc, int gridsize, int slice_size, double *yc, double *dyc, double *r_y, double *r_dy)
{
    // the ALL processes will send, and the root will receive from ALL processes (even itself)
    MPI_Request y_send, dy_send, *y_recv = NULL, *dy_recv = NULL;

    // root will receive from all processes, even itself
    if (rank == ROOT)
    {
        y_recv = malloc(sizeof(MPI_Request) * numproc);
        dy_recv = malloc(sizeof(MPI_Request) * numproc);
        if (y_recv == NULL || dy_recv == NULL)
        {
            MPI_Finalize();
            exit(1);
        }

        // have to track the "extra" datapoints that need to be recieved
        // due to uneven sharing of data
        const int rem = gridsize % numproc;
        const int base_count = gridsize / numproc;
        int extra = 0;
        for (int i = 0; i < numproc; i++)
        {
            MPI_Irecv(r_y + (i * base_count) + extra, slice_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, y_recv + i);
            MPI_Irecv(r_dy + (i * base_count) + extra, slice_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, dy_recv + i);
            extra += rem > i ? 1 : 0;
        }
    }

    MPI_Isend(yc + 1, slice_size, MPI_DOUBLE, ROOT, 0, MPI_COMM_WORLD, &y_send);
    MPI_Isend(dyc + 1, slice_size, MPI_DOUBLE, ROOT, 0, MPI_COMM_WORLD, &dy_send);
    // MPI_Request_free(&y_send);
    // MPI_Request_free(&dy_send);

    // wait for recieves to complete and cleanup allocated buffers
    if (rank == ROOT)
    {
        MPI_Waitall(numproc, y_recv, MPI_STATUS_IGNORE);
        MPI_Waitall(numproc, dy_recv, MPI_STATUS_IGNORE);
        free(y_recv);
        free(dy_recv);
    }

    MPI_Wait(&y_send, MPI_STATUS_IGNORE);
    MPI_Wait(&dy_send, MPI_STATUS_IGNORE);
}

/* END Gather functions ******************************************************/

// prints out the function and its derivative to a file
void print_function_data(int np, double *x, double *y, double *dydx)
{
    int i;

    char filename[1024];
    sprintf(filename, "fn-%d.dat", np);

    FILE *fp = fopen(filename, "w");

    for (i = 0; i < np; i++)
    {
        fprintf(fp, "%f %f %f\n", x[i], y[i], dydx[i]);
    }

    fclose(fp);
}
