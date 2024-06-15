#include "common.h"

void client(int rank, int world_size, char *processor_name) {
  printf("------- Inicializando Cliente -------\n");
  printf("Rank: %d\nProcessor: %s\n", rank, processor_name);
  printf("-------------------------------------\n\n");
  MPI_Datatype MPI_DB_MESSAGE_TYPE;
  create_message_t_type(&MPI_DB_MESSAGE_TYPE);

  int *client_ranks, client_size;
  get_client_ranks(world_size, &client_ranks, &client_size);

  if (rank == client_ranks[0]) {
    // TODO: enviar requisição para o load balancer
    // TODO: receber resposta da replica
    message_t message1 = new_write_message(rank, "the killers", "mr brightside");
    // message_t message2 = new_write_message(rank, "paramore", "still into you");
    // message_t message3 = new_write_message(rank, "the strokes", "last nite");
    message_t message4 = new_read_message(rank, "the killers");
    // message_t message5 = new_read_message(rank, "paramore");
    // message_t message6 = new_write_message(rank, "paramore", "the only exception");
    // message_t message7 = new_read_message(rank, "paramore");
    MPI_Send(&message1, 1, MPI_DB_MESSAGE_TYPE, LOAD_BALANCER_RANK, WRITE_MESSAGE_TAG, MPI_COMM_WORLD);
    // MPI_Send(&message2, 1, MPI_DB_MESSAGE_TYPE, LOAD_BALANCER_RANK, WRITE_MESSAGE_TAG, MPI_COMM_WORLD);
    // MPI_Send(&message3, 1, MPI_DB_MESSAGE_TYPE, LOAD_BALANCER_RANK, WRITE_MESSAGE_TAG, MPI_COMM_WORLD);
    MPI_Send(&message4, 1, MPI_DB_MESSAGE_TYPE, LOAD_BALANCER_RANK, READ_MESSAGE_TAG, MPI_COMM_WORLD);
    // MPI_Send(&message5, 1, MPI_DB_MESSAGE_TYPE, LOAD_BALANCER_RANK, READ_MESSAGE_TAG, MPI_COMM_WORLD);
    // MPI_Send(&message6, 1, MPI_DB_MESSAGE_TYPE, LOAD_BALANCER_RANK, WRITE_MESSAGE_TAG, MPI_COMM_WORLD);
    // MPI_Send(&message7, 1, MPI_DB_MESSAGE_TYPE, LOAD_BALANCER_RANK, READ_MESSAGE_TAG, MPI_COMM_WORLD);
  }

  MPI_Type_free(&MPI_DB_MESSAGE_TYPE);
}
