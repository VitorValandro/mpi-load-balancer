#include "common.h"

void load_balancer(int rank, int world_size, char *processor_name) {
  printf("Load Balancer Rank %d, processor %s\n", rank, processor_name);
  int *client_ranks, client_size, *replica_ranks, replica_size;
  get_client_ranks(world_size, &client_ranks, &client_size);
  get_replica_ranks(world_size, &replica_ranks, &replica_size);

  if (client_size == 0 || replica_size == 0) {
    fprintf(stderr, "Erro ao obter os ranks dos clientes e réplicas\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
    return;
  }

  // TODO: receber dos clientes, enviar para as réplicas
  // leituras: round-robin
  // escritas: replicação em broadcast

  free(client_ranks);
  free(replica_ranks);
}
