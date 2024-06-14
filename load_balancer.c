#include "common.h"

void load_balancer(int rank, int world_size, char *processor_name) {
  printf("Load Balancer Rank %d, processor %s\n", rank, processor_name);
  MPI_Datatype MPI_DB_MESSAGE_TYPE;
  create_message_t_type(&MPI_DB_MESSAGE_TYPE);

  int *client_ranks, client_size, *replica_ranks, replica_size;
  get_client_ranks(world_size, &client_ranks, &client_size);
  get_replica_ranks(world_size, &replica_ranks, &replica_size);

  if (client_size == 0 || replica_size == 0) {
    fprintf(stderr, "Erro ao obter os ranks dos clientes e réplicas\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
    return;
  }

  message_t message = new_write_message(rank, "key", "value");
  MPI_Send(&message, 1, MPI_DB_MESSAGE_TYPE, 4, 2, MPI_COMM_WORLD);

  // TODO: receber dos clientes, enviar para as réplicas
  // leituras: round-robin
  // escritas: replicação em broadcast

  // while (1) {
  //   MPI_Recv(&msg, sizeof(message_t), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  // }

  free(client_ranks);
  free(replica_ranks);
  MPI_Type_free(&MPI_DB_MESSAGE_TYPE);
}
