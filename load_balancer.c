#include "common.h"

void load_balancer(int rank, int world_size, char *processor_name) {
  printf("------- Inicializando Load Balancer -------\n");
  printf("Rank: %d\nProcessor: %s\n", rank, processor_name);
  printf("-------------------------------------\n\n");
  MPI_Datatype MPI_DB_MESSAGE_TYPE;
  create_message_t_type(&MPI_DB_MESSAGE_TYPE);

  int *client_ranks, client_size, *replica_ranks, replica_size;
  get_client_ranks(world_size, &client_ranks, &client_size);
  get_replica_ranks(world_size, &replica_ranks, &replica_size);

  if (client_size == 0 || replica_size == 0) {
    fprintf(stderr, "Erro ao obter os ranks dos clientes e réplicas\n\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
    return;
  }

  message_t message;
  MPI_Status status;
  int current_replica_rank = replica_ranks[0];
  while (1) {
    MPI_Recv(&message, sizeof(message_t), MPI_DB_MESSAGE_TYPE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    if (status.MPI_TAG == READ_MESSAGE_TAG) {
      // Envia a mensagem para a réplica atual
      MPI_Send(&message, 1, MPI_DB_MESSAGE_TYPE, current_replica_rank, READ_MESSAGE_TAG, MPI_COMM_WORLD);
      printf("-------------------------------------\n"
             "Load Balancer recebeu um pedido de leitura do cliente %d\n"
             "Enviando pedido para réplica `%d` para buscar valor da chave `%s`\n"
             "-------------------------------------\n\n",
             message.client_rank, current_replica_rank, message.key);
      // Atualiza para a próxima réplica usando round-robin
      current_replica_rank = replica_ranks[(current_replica_rank + 1) % replica_size];
    } else if (status.MPI_TAG == WRITE_MESSAGE_TAG) {
      // Envia a mensagem para todas as réplicas
      printf("-------------------------------------\n"
             "Load Balancer recebeu um pedido de escrita do cliente %d\n"
             "Iniciando difusão do pedido de escrita de `%s` na chave `%s` para todas as réplicas\n"
             "-------------------------------------\n\n",
             message.client_rank, message.value, message.key);
      for (int i = 0; i < replica_size; i++) {
        MPI_Send(&message, 1, MPI_DB_MESSAGE_TYPE, replica_ranks[i], WRITE_MESSAGE_TAG, MPI_COMM_WORLD);
      }
      printf("Load Balancer difundiu o pedido de escrita de `%s` na chave `%s` para todas as réplicas\n\n",
             message.value, message.key);
    }
  }

  free(client_ranks);
  free(replica_ranks);
  MPI_Type_free(&MPI_DB_MESSAGE_TYPE);
}
