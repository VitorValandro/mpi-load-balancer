#include "common.h"

void client(int rank, int world_size, char *processor_name) {
  printf("Client Rank %d, processor %s\n", rank, processor_name);
  MPI_Datatype MPI_MESSAGE_WRITE_TYPE, MPI_MESSAGE_READ_TYPE;
  create_message_write_t_type(&MPI_MESSAGE_WRITE_TYPE);
  create_message_read_t_type(&MPI_MESSAGE_READ_TYPE);

  int load_balancer_rank = LOAD_BALANCER_RANK;

  // TODO: enviar requisição para o load balancer
  // TODO: receber resposta da replica

  MPI_Status status;
  message_write_t write_message;
  MPI_Recv(&write_message, 1, MPI_MESSAGE_WRITE_TYPE, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status);
  fprintf(stderr, "------------------\n");
  fprintf(stderr, "client %d\n", write_message.client_rank);
  fprintf(stderr, "key %s\n", write_message.key);
  fprintf(stderr, "value %s\n", write_message.value);
  fprintf(stderr, "------------------\n");
  fprintf(stderr, "tag %d\n", status.MPI_TAG);
}
