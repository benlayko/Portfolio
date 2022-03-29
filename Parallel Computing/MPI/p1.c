/******************************************************************************
 * FILE: p1.c
 * DESCRIPTION:
 *   HW1 individual project. Computes the average time to send messages of 
 *   different sizes.
 * bjlayko Ben J. Layko
 * LAST REVISED: 01/20/22
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <sys/time.h>
#include <math.h>

/* This is the root process */
#define  ROOT       0

int main (int argc, char *argv[])
{

        /* process information */
        int numproc, rank, len;

        /* current process hostname */
        char  hostname[MPI_MAX_PROCESSOR_NAME];

        /* initialize MPI */
        MPI_Init(&argc, &argv);

        /* get the number of procs in the comm */
        MPI_Comm_size(MPI_COMM_WORLD, &numproc);

        /* get my rank in the comm */
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);

        /* get some information about the host I'm running on */
        MPI_Get_processor_name(hostname, &len);
        
        //All even rank processor will send a message to the rank + 1 above it.
        if(rank % 2 == 0){
            //Define the number of bytes in a first message
            long bytes = 32768;
            while(bytes <= 2097152){
                //Create an array to hold the different times for a message
                double times[10];
                //Create data to send
                void * data = (void *)malloc(bytes * sizeof(char));
                //Time and send the message 10 times
                MPI_Status status;
                for(int i = 0; i < 10; i++){
                    struct timeval start_time;
                    struct timeval end_time;
                    //Start timer
                    gettimeofday(&start_time, NULL);
                    //Send message
                    MPI_Send(data, bytes, MPI_UNSIGNED_CHAR, rank + 1, 0, MPI_COMM_WORLD);
                    MPI_Recv(NULL, 0, MPI_BYTE, rank + 1, 0, MPI_COMM_WORLD, &status);
                    //End timer
                    gettimeofday(&end_time, NULL);
                    times[i] = (end_time.tv_usec - start_time.tv_usec) * 1.0E-6;
                }
                free(data);
                
                //If the rank is 0, output the times
                if(rank == 0){
                    double sd = 0;
                    double average = 0;
                    //Calculate the average
                    for(int i = 1; i < 10; i++){
                        average += times[i];
                    }
                    average = average / 9;
                    //Calculate the standard deviation
                    for(int i = 1; i < 10; i++){
                        sd += pow(times[i] - average,2);
                    }
                    sd = sd / 9;
                    sd = sqrt(sd);
                    
                    printf("%ld %f %f\n", bytes, average, sd);
                }
                bytes = bytes * 2;
            }
            
        } else {
            MPI_Status status;
            //This section is for the receiving processors
            long bytes = 32768;
            while(bytes <= 2097152){
                for(int i = 0; i < 10; i++){
                    //Create a buffer
                    void * data = (void *)malloc(bytes * sizeof(char));
                    //Recieve the data
                    MPI_Recv(data, bytes, MPI_UNSIGNED_CHAR, rank - 1, 0, MPI_COMM_WORLD, &status);
                    MPI_Send(NULL, 0, MPI_BYTE, rank - 1, 0, MPI_COMM_WORLD);
                    free(data);
                    
                }
                bytes = bytes * 2;
            }
        }            

        
        

        /* graceful exit */
        MPI_Finalize();
}
