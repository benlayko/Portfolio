CC = mpicc
CFLAGS = -O3 -lm

all: p2 p2_mpi

p2: p2.c p2_func.c

p2_mpi: p2_mpi.c p2_func.c
