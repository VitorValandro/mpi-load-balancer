#ifndef COMMON_H
#define COMMON_H

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOAD_BALANCER_RANK 0
#define READ_MESSAGE_TAG 1
#define WRITE_MESSAGE_TAG 2
#define REPLY_MESSAGE_TAG 3
#define TERMINATE_MESSAGE_TAG 4
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

typedef struct {
  char operation[6];
  char key[MAX_KEY_VALUE_LENGTH];
  char value[MAX_KEY_VALUE_LENGTH];
} operation_t;

void load_balancer(int rank, int world_size, char *processor_name);
void replica(int rank, int world_size, char *processor_name);
void client(int rank, int world_size, char *processor_name, operation_t *operations, int operation_count);

void calculate_units(int world_size, int *replica_units, int *client_units);
void get_replica_ranks(int world_size, int **ranks, int *size);
void get_client_ranks(int world_size, int **ranks, int *size);

void create_message_t_type(MPI_Datatype *mpi_message_t_type);

message_t new_write_message(int client_rank, char *key, char *value);
message_t new_read_message(int client_rank, char *key);
message_t new_reply_message(int client_rank, char *key, const char *value);
message_t new_terminate_message(int client_rank);

void parse_operations(const char *filename, operation_t **operations, int *operation_count);

#endif // COMMON_H
