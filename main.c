#include "common.h"

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);

  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  int N = 3; // Number of clients
  int M = 2; // Number of file replicas

  if (world_rank == 0)
  {
    load_balancer(world_rank);
  }
  else if (world_rank > 0 && world_rank <= N)
  {
    client(world_rank);
  }
  else
  {
    file_replica(world_rank);
  }

  MPI_Finalize();
  return 0;
}
