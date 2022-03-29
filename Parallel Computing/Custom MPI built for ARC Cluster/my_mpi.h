/**
 * FILE: my_mpi.h
 * DESCRIPTION: Header file for the custom MPI implementation for homework 2
 *  AUTHOR:
 *  bjlayko Benjamin J Layko
 */

//Defining the MPI datatypes that are used as parameters
#define MPI_COMM_WORLD 1
#define MPI_MAX_PROCESSOR_NAME 255
//Note that data types will be treated as the size of the datatype they represent
#define MPI_Datatype int
#define MPI_Comm int
#define MPI_CHAR 1
#define MPI_SIGNED_CHAR 1
#define MPI_UNSIGNED_CHAR 1
#define MPI_SHORT 2
#define	MPI_UNSIGNED_SHORT 2
#define	MPI_INT 4
#define	MPI_UNSIGNED 4
#define	MPI_LONG long 8
#define	MPI_UNSIGNED_LONG 8
#define	MPI_LONG_LONG_INT 8
#define	MPI_FLOAT 4
#define	MPI_DOUBLE 8
#define	MPI_LONG_DOUBLE 10
#define	MPI_BYTE 1

#define MPI_Status int

//Custom version of the MPI_Init function. Called by main thread to start the mpi
int MPI_Init(int *argc, char ***argv);

//Custom version of the MPI_Finalize function. All processes call this before exiting
int MPI_Finalize( void );

//Custom version of the MPI_Comm_size function. Determines the size of the communicator
int MPI_Comm_size( MPI_Comm comm, int *size ); 

//Custom version of the MPI_Comm_rank function. Determines the rank of the calling process in the communicator
int MPI_Comm_rank(MPI_Comm comm, int *rank);

//Custom version of the MPI_Get_processor_name function. Gets the name of the processor
int MPI_Get_processor_name( char *name, int *resultlen );

//Performs a blocking send
int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);

//Performs a blocking recieve
int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);

//Creates a barrier that all calls must get to before the program can continue
int MPI_Barrier(MPI_Comm comm);