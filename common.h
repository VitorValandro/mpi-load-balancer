#ifndef COMMON_H
#define COMMON_H

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define LOAD_BALANCER_RANK 0
#define TAG_MESSAGE_READ 1
#define TAG_MESSAGE_WRITE 2
#define MAX_KEY_VALUE_LENGTH 100
#define WRITE_MESSAGE_TYPE 1
#define READ_MESSAGE_TYPE 2
#define REPLY_MESSAGE_TYPE 3

typedef struct {
  int client_rank;
  int message_type; // 1 pra escrita, 2 pra leitura e 3 pra resposta da leitura
  char key[MAX_KEY_VALUE_LENGTH];
  char value[MAX_KEY_VALUE_LENGTH]; // usado pra escrita e resposta da leitura
} message_t;

void load_balancer(int rank, int world_size, char *processor_name);
void replica(int rank, int world_size, char *processor_name);
void client(int rank, int world_size, char *processor_name);

void calculate_units(int world_size, int *replica_units, int *client_units);
void get_replica_ranks(int world_size, int **ranks, int *size);
void get_client_ranks(int world_size, int **ranks, int *size);

void create_message_t_type(MPI_Datatype *mpi_message_t_type);

message_t new_write_message(int client_rank, char *key, char *value);
message_t new_read_message(int client_rank, char *key);
message_t new_reply_message(int client_rank, char *key, char *value);

#endif // COMMON_H
