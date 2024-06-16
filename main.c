#include "common.h"

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  // Garante que o número de processos é suficiente para executar o programa
  if (world_size < 6) {
    fprintf(stderr, "World size deve ser pelo menos 6 para este programa\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
    return -1;
  }

  MPI_Datatype MPI_DB_MESSAGE_TYPE;
  create_message_t_type(&MPI_DB_MESSAGE_TYPE);

  // Calcula o número de réplicas e clientes
  int replica_units, client_units;
  calculate_units(world_size, &replica_units, &client_units);

  // Obtém o identificador do host que está executando o processo
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);

  // Inicializa o load balancer, réplicas e clientes com base no cálculo de unidades
  if (world_rank == 0) {
    load_balancer(world_rank, world_size, processor_name);
  } else if (world_rank <= replica_units) {
    replica(world_rank, world_size, processor_name);
  } else if (world_rank > replica_units && world_rank <= (replica_units + client_units)) {
    char filename[50];
    snprintf(filename, sizeof(filename), "client_operations/operations%d.txt", world_rank - replica_units);

    operation_t *operations;
    int operation_count;
    parse_operations(filename, &operations, &operation_count);

    client(world_rank, world_size, processor_name, operations, operation_count);
  }

  MPI_Type_free(&MPI_DB_MESSAGE_TYPE);
  MPI_Finalize();
  return 0;
}
