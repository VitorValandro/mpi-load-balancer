#include "common.h"

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);

  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  // Ensure minimum world size for 1 load balancer, 2 replicas, and 3 clients
  if (world_size < 6)
  {
    fprintf(stderr, "World size deve ser pelo menos 6 para este programa\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  // Calculate the number of replicas and clients based on the 2:3 ratio
  // Subtract 1 from world_size for the load balancer
  int total_units = world_size - 1;
  int unit = 5; // Sum of ratio parts (2 for replicas + 3 for clients)
  int replica_units = 2 * (total_units / unit);
  int client_units = 3 * (total_units / unit);

  // Get the name of the processor
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);

  // Adjust if there are remaining units due to rounding
  int remaining_units = total_units - (replica_units + client_units);
  while (remaining_units > 0)
  {
    if (replica_units <= client_units)
    {
      replica_units++;
    }
    else
    {
      client_units++;
    }
    remaining_units--;
  }

  // Assign roles based on calculated numbers
  if (world_rank == 0)
  {
    load_balancer(world_rank, processor_name);
  }
  else if (world_rank <= replica_units)
  {
    replica(world_rank, processor_name);
  }
  else if (world_rank > replica_units && world_rank <= (replica_units + client_units))
  {
    client(world_rank, processor_name);
  }

  MPI_Finalize();
  return 0;
}