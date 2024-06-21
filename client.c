#include "common.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct {
  int rank;
  int world_size;
  char *processor_name;
  operation_t *operations;
  int operation_count;
  MPI_Datatype *MPI_DB_MESSAGE_TYPE;
  int read_count;
} thread_data_t;

void *send_operations(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
  int rank = data->rank;
  MPI_Datatype MPI_DB_MESSAGE_TYPE = *(data->MPI_DB_MESSAGE_TYPE);

  srand(time(NULL)); // Inicializa a semente do gerador de números aleatórios

  for (int i = 0; i < data->operation_count; i++) {
    if (strcmp(data->operations[i].operation, "READ") == 0) {
      printf("-------------------------------------\n"
             "Cliente %d: READ key `%s`\n"
             "-------------------------------------\n\n",
             rank, data->operations[i].key);
      message_t read_message = new_read_message(rank, data->operations[i].key);
      MPI_Send(&read_message, 1, MPI_DB_MESSAGE_TYPE, LOAD_BALANCER_RANK, READ_MESSAGE_TAG, MPI_COMM_WORLD);
    } else if (strcmp(data->operations[i].operation, "WRITE") == 0) {
      printf("-------------------------------------\n"
             "Cliente %d: WRITE key `%s` value `%s`\n"
             "-------------------------------------\n\n",
             rank, data->operations[i].key, data->operations[i].value);
      message_t write_message = new_write_message(rank, data->operations[i].key, data->operations[i].value);
      MPI_Send(&write_message, 1, MPI_DB_MESSAGE_TYPE, LOAD_BALANCER_RANK, WRITE_MESSAGE_TAG, MPI_COMM_WORLD);
    }

    // Gerar um atraso aleatório entre 1 e 2 segundos
    int delay = (rand() % 1000001) + 1000000; // 1 segundo = 1.000.000 microssegundos
    usleep(delay);
  }

  pthread_exit(NULL);
}

void *receive_responses(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
  int rank = data->rank;
  MPI_Datatype MPI_DB_MESSAGE_TYPE = *(data->MPI_DB_MESSAGE_TYPE);

  int read_responses_received = 0;
  message_t response_message;
  while (read_responses_received < data->read_count) {
    MPI_Status status;
    MPI_Recv(&response_message, 1, MPI_DB_MESSAGE_TYPE, MPI_ANY_SOURCE, REPLY_MESSAGE_TAG, MPI_COMM_WORLD, &status);
    printf("-------------------------------------\n"
           "Cliente %d: READ REPLY key `%s` value `%s`\n"
           "-------------------------------------\n\n",
           rank, response_message.key, response_message.value);

    read_responses_received++;
  }

  pthread_exit(NULL);
}

void client(int rank, int world_size, char *processor_name, operation_t *operations, int operation_count) {
  printf("------- Inicializando Cliente -------\n");
  printf("Rank: %d\nProcessor: %s\n", rank, processor_name);
  printf("-------------------------------------\n\n");
  MPI_Datatype MPI_DB_MESSAGE_TYPE;
  create_message_t_type(&MPI_DB_MESSAGE_TYPE);

  // Conta o número de operações READ
  int read_count = 0;
  for (int i = 0; i < operation_count; i++) {
    if (strcmp(operations[i].operation, "READ") == 0) {
      read_count++;
    }
  }

  thread_data_t data;
  data.rank = rank;
  data.world_size = world_size;
  data.processor_name = processor_name;
  data.operations = operations;
  data.operation_count = operation_count;
  data.MPI_DB_MESSAGE_TYPE = &MPI_DB_MESSAGE_TYPE;
  data.read_count = read_count;

  pthread_t send_thread, receive_thread;

  // Implementa o padrão Fork Join para enviar e receber mensagens concorrentemente
  pthread_create(&send_thread, NULL, send_operations, (void *)&data);
  pthread_create(&receive_thread, NULL, receive_responses, (void *)&data);

  pthread_join(send_thread, NULL);
  pthread_join(receive_thread, NULL);

  message_t terminate_message = new_terminate_message(rank);
  MPI_Send(&terminate_message, 1, MPI_DB_MESSAGE_TYPE, LOAD_BALANCER_RANK, TERMINATE_MESSAGE_TAG, MPI_COMM_WORLD);

  printf("------- Encerrando cliente %d -------\n\n", rank);

  MPI_Type_free(&MPI_DB_MESSAGE_TYPE);
  MPI_Finalize();
}
