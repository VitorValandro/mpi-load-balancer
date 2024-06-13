#include "common.h"

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  // Ensure minimum world size for 1 load balancer, 2 replicas, and 3 clients
  if (world_size < 6) {
    fprintf(stderr, "World size deve ser pelo menos 6 para este programa\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  // Calculate the number of replicas and clients
  int replica_units, client_units;
  calculate_units(world_size, &replica_units, &client_units);

  // Get the name of the processor
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);

  // Assign roles based on calculated numbers
  if (world_rank == 0) {
    load_balancer(world_rank, world_size, processor_name);
  } else if (world_rank <= replica_units) {
    replica(world_rank, world_size, processor_name);
  } else if (world_rank > replica_units && world_rank <= (replica_units + client_units)) {
    client(world_rank, world_size, processor_name);
  }

  MPI_Finalize();
  return 0;
}
