#include "common.h"
#include "key_value_db.h"
#include <stdio.h>

void replica(int rank, int world_size, char *processor_name) {
  printf("------- Inicializando réplica -------\n");
  printf("Rank: %d\nProcessor: %s\n", rank, processor_name);
  printf("-------------------------------------\n\n");
  MPI_Datatype MPI_DB_MESSAGE_TYPE;
  create_message_t_type(&MPI_DB_MESSAGE_TYPE);

  KeyValueDB db;
  initDB(&db);

  MPI_Status status;
  message_t msg;

  while (1) {
    printf("Replica %d esperando mensagens do load balancer...\n\n", rank);
    MPI_Recv(&msg, sizeof(message_t), MPI_DB_MESSAGE_TYPE, LOAD_BALANCER_RANK, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    if (msg.message_type == WRITE_MESSAGE_TYPE) {
      upsertEntry(&db, msg.key, msg.value);
      printf("-------------------------------------\n"
             "Replica %d recebeu uma mensagem de escrita do load balancer\n"
             "Replica %d inseriu ou atualizou a chave `%s` com o valor `%s`\n"
             "-------------------------------------\n\n",
             rank, rank, msg.key, msg.value);
    } else if (msg.message_type == READ_MESSAGE_TYPE) {
      const char *value = getValue(&db, msg.key);
      message_t reply = new_reply_message(msg.client_rank, msg.key, value);
      MPI_Send(&reply, 1, MPI_DB_MESSAGE_TYPE, msg.client_rank, REPLY_MESSAGE_TAG, MPI_COMM_WORLD);
      printf("-------------------------------------\n"
             "Replica %d recebeu uma mensagem de leitura load balancer\n"
             "Replica %d enviou o valor `%s` para a chave `%s` para o cliente %d\n"
             "-------------------------------------\n\n",
             rank, rank, value, msg.key, msg.client_rank);
    }
  }

  printf("------- Encerrando réplica %d -------\n\n", rank);

  freeDB(&db);
  MPI_Finalize();
}