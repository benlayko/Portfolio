/**
 * FILE: my_mpi.c
 * DESCRIPTION: Custom MPI library that will be able to emulate MPI using sockets and multithreading. 
 * The socket code was modified from the sample code provided at : https://www.linuxhowtos.org/data/6/server.c
 *  AUTHOR:
 *  bjlayko Benjamin J Layko
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include "my_mpi.h"

#define PORTNO 9999
int recvSize;
int rankOfNode;
int nodeCount;
char **address_buff;
//Socket init material for receiving
int server_fd, new_socket;
struct sockaddr_in address;
int opt = 1;
int addrlen = sizeof(address);

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
//This is where the message passing will be initalized and the threads will be created
int MPI_Init(int *argc, char ***argv){
    if(*argc != 3){
        fprintf(stderr, "Usage: %s {node rank} {number of nodes}\n", *argv[0]);
        exit(1);
    }     
    char * arg1 = argv[0][1];
    char * arg2 = argv[0][2];
    rankOfNode = atoi(arg1);
    nodeCount = atoi(arg2);

    FILE *fp;

    fp = fopen("nodefile.txt", "r");

    address_buff = (char **)malloc(sizeof(char *) * nodeCount);
    for(int i = 0; i < nodeCount; i++){
        address_buff[i] = (char *)malloc(sizeof(char) * 5);
        fgets(address_buff[i], 10, (FILE*) fp);
    }
    for(int i = 0; i < nodeCount; i++){
        for(int j = 0; j < 5; j++){
            if(address_buff[i][j] == '\n'){
                address_buff[i][j] = '\0';
            }
        }
    }
    
    fclose(fp);
    
    
       
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Forcefully attaching socket to the port PORTNO
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    //Do not specify a port so that it finds an open one
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = 0;
       
    // Forcefully attaching socket to the port PORTNO
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    //Get the port number from the struct
    socklen_t len = sizeof(address);
    getsockname(server_fd, (struct sockaddr *)&address, &len);

    //Create a file that holds the port number for the sender
    char filename[51];
    strcpy(filename, "port-");
    char * rankString = (char*)malloc(sizeof(char) * 3);
    sprintf(rankString, "%d", rankOfNode);
    strcat(filename, rankString);
    strcat(filename, ".txt");
    FILE *fp2;
    fp2 = fopen(filename, "w");
    fprintf(fp2, "%d\n", address.sin_port);
    fclose(fp2);

    //Listen for the information
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return 1;
    //Figure out how to get host and port number to pass to create server
}

//This will close the multiple threads
int MPI_Finalize( void ) {
    MPI_Barrier(0);
    for(int i = 0; i < nodeCount; i++){
        free(address_buff[i]);
    }
    free(address_buff);
    close(server_fd);

    char filename[51];
    strcpy(filename, "port-");
    char * rankString = (char*)malloc(sizeof(char) * 3);
    sprintf(rankString, "%d", rankOfNode);
    strcat(filename, rankString);
    strcat(filename, ".txt");
    remove(filename);
    return 1;
}

//Custom version of the MPI_Comm_size function. Determines the size of the communicator
int MPI_Comm_size( MPI_Comm comm, int *size ){
    *size = nodeCount;
    return nodeCount;
}    

//Custom version of the MPI_Comm_rank function. Determines the rank of the calling process in the communicator
int MPI_Comm_rank(MPI_Comm comm, int *rank){
    *rank = rankOfNode;
    return rankOfNode;
}

//Custom version of the MPI_Get_processor_name function. Gets the name of the processor
int MPI_Get_processor_name( char *name, int *resultlen ){
    name = address_buff[rankOfNode];
    return 1;
}

//Performs a blocking send
int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){  
    //Socket init code
    int sock = 0;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    
    //This gets the host name of the destination from the addres buffer
    const char * hostName = address_buff[dest];
    server = gethostbyname(hostName);
    //It is possible the host doesn't exist yet
    while (server == NULL) {
        sleep(1);
        fprintf(stderr,"ERROR, no such host\n");
        server = gethostbyname(hostName);
    }

    //Set the server address to 0
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    
    //This code is used to open the file that the reciever has created to communicate the correct port number
    char filename[51];
    strcpy(filename, "port-");
    char * destString = (char*)malloc(sizeof(char) * 3);
    sprintf(destString, "%d", dest);
    strcat(filename, destString);
    strcat(filename, ".txt");
    FILE *fp;
    fp = fopen(filename, "r");
    while(fp == NULL){
        sleep(1);
        fp = fopen(filename, "r");
    }
    char * asciiPort =  (char *)malloc(sizeof(char) * 10);
    fgets(asciiPort, 10, fp);
    int port = atoi(asciiPort);
    fclose(fp);

    //Set the port number here
    serv_addr.sin_port = port;
    
    //Try to connect
    while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        sleep(1);
    }
    //Send the message
    send(sock , buf , datatype * count * sizeof(char) , 0 );
    close(sock);
    return 0;
}

//Performs a blocking recieve
int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status){
    //Accept the connection
    int new_socket;
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                       (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    //Read the information in
    read( new_socket , buf, sizeof(char) * count * datatype);
    close(new_socket);
    
    return 0;
}

int MPI_Barrier(MPI_Comm comm){
    if(rankOfNode == 0){
        for(int i = 1; i < nodeCount; i++){
            int sock = 0;
            struct sockaddr_in serv_addr;
            struct hostent *server;
            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                printf("\n Socket creation error \n");
                return -1;
            }

            //This gets the host name of the destination from the addres buffer
            const char * hostName = address_buff[i];
            server = gethostbyname(hostName);
            //It is possible the host doesn't exist yet
            while (server == NULL) {
                sleep(1);
                fprintf(stderr,"ERROR, no such host\n");
                server = gethostbyname(hostName);
            }

            //Set the server address to 0
            bzero((char *) &serv_addr, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            bcopy((char *)server->h_addr, 
                 (char *)&serv_addr.sin_addr.s_addr,
                 server->h_length);

            //This code is used to open the file that the reciever has created to communicate the correct port number
            char filename[51];
            strcpy(filename, "port-");
            char * destString = (char*)malloc(sizeof(char) * 3);
            sprintf(destString, "%d", i);
            strcat(filename, destString);
            strcat(filename, ".txt");
            FILE *fp;
            fp = fopen(filename, "r");
            while(fp == NULL){
                sleep(1);
                fp = fopen(filename, "r");
            }
            char * asciiPort =  (char *)malloc(sizeof(char) * 10);
            fgets(asciiPort, 10, fp);
            int port = atoi(asciiPort);
            fclose(fp);

            //Set the port number here
            serv_addr.sin_port = port;

            //Try to connect
            while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            {
                sleep(1);
            }
            //Send the message
            int * confirm = (int *)malloc(sizeof(int));
            *confirm = 1;
            send(sock , confirm , sizeof(int) , 0 );
            close(sock);
        }
    } else {
        //Accept the connection
        int new_socket;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                           (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        //Read the information in
        read( new_socket , NULL, sizeof(int));
        close(new_socket);
    }
    return 0;
}