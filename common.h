#ifndef COMMON_H
#define COMMON_H

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define LOAD_BALANCER_RANK 0

void load_balancer(int rank, int world_size, char *processor_name);
void replica(int rank, int world_size, char *processor_name);
void client(int rank, int world_size, char *processor_name);

void calculate_units(int world_size, int *replica_units, int *client_units);
void get_replica_ranks(int world_size, int **ranks, int *size);
void get_client_ranks(int world_size, int **ranks, int *size);

#endif // COMMON_H
