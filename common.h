#ifndef COMMON_H
#define COMMON_H

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define LOAD_BALANCER 0
#define CLIENT 1
#define FILE_REPLICA 2

void load_balancer();
void client();
void file_replica();

#endif // COMMON_H
