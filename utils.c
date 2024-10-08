#include "common.h"
#include <stdlib.h>

void calculate_units(int world_size, int *replica_units, int *client_units) {
  int total_units = world_size - 1;
  int unit = 5; // Soma dos pesos de replica e cliente (2:3)
  *replica_units = 2 * (total_units / unit);
  *client_units = 3 * (total_units / unit);

  // Ajusta se houver unidades restantes devido ao arredondamento
  int remaining_units = total_units - (*replica_units + *client_units);
  while (remaining_units > 0) {
    if (*replica_units <= *client_units) {
      (*replica_units)++;
    } else {
      (*client_units)++;
    }
    remaining_units--;
  }
}

void get_replica_ranks(int world_size, int **ranks, int *size) {
  int replica_units, client_units;
  calculate_units(world_size, &replica_units, &client_units);
  *size = replica_units; // Define o tamanho
  // Aloca memória para os ranks das réplicas
  *ranks = malloc(replica_units * sizeof(int));
  if (*ranks != NULL) {
    for (int i = 0; i < replica_units; i++) {
      // Preenche o array com os ranks das réplicas
      // O rank da réplica é i + 1 (começa em 1, rank 0 é o load balancer)
      (*ranks)[i] = i + 1;
    }
  }
}

void get_client_ranks(int world_size, int **ranks, int *size) {
  int replica_units, client_units;
  calculate_units(world_size, &replica_units, &client_units);
  *size = client_units; // Define o tamanho
  // Aloca memória para os ranks dos clientes
  *ranks = malloc(client_units * sizeof(int));
  if (*ranks != NULL) {
    for (int i = 0; i < client_units; i++) {
      // Preenche o array com os ranks dos clientes
      // Os ranks dos clientes começam a partir das réplicas
      (*ranks)[i] = replica_units + i + 1;
    }
  }
}

void create_message_t_type(MPI_Datatype *mpi_message_t_type) {
  // Define o número de elementos
  const int nitems = 4;

  // Define o tamanho de cada elemento
  int blocklengths[4] = {1, 1, MAX_KEY_VALUE_LENGTH, MAX_KEY_VALUE_LENGTH};

  // Define os offsets de cada elemento
  MPI_Aint offsets[4];
  offsets[0] = offsetof(message_t, client_rank);
  offsets[1] = offsetof(message_t, message_type);
  offsets[2] = offsetof(message_t, key);
  offsets[3] = offsetof(message_t, value);

  // Define os tipos de dados
  MPI_Datatype types[4] = {MPI_INT, MPI_INT, MPI_CHAR, MPI_CHAR};

  // Cria o custom datatype
  MPI_Type_create_struct(nitems, blocklengths, offsets, types, mpi_message_t_type);

  // Commita o custom datatype
  MPI_Type_commit(mpi_message_t_type);
}

message_t new_write_message(int client_rank, char *key, char *value) {
  message_t message;
  message.client_rank = client_rank;
  message.message_type = WRITE_MESSAGE_TYPE; // indica que é uma mensagem de escrita
  snprintf(message.key, MAX_KEY_VALUE_LENGTH, "%s", key);
  snprintf(message.value, MAX_KEY_VALUE_LENGTH, "%s", value);
  return message;
}

message_t new_read_message(int client_rank, char *key) {
  message_t message;
  message.client_rank = client_rank;
  message.message_type = READ_MESSAGE_TYPE; // Indica que é uma mensagem de leitura
  snprintf(message.key, MAX_KEY_VALUE_LENGTH, "%s", key);
  message.value[0] = '\0'; // Inicializa o valor vazio
  return message;
}

message_t new_reply_message(int client_rank, char *key, const char *value) {
  message_t message;
  message.client_rank = client_rank;
  message.message_type = REPLY_MESSAGE_TYPE; // Indica que é uma resposta de retorno
  snprintf(message.key, MAX_KEY_VALUE_LENGTH, "%s", key);
  snprintf(message.value, MAX_KEY_VALUE_LENGTH, "%s", value);
  return message;
}

message_t new_terminate_message(int client_rank) {
  message_t message;
  message.client_rank = client_rank;
  message.message_type = TERMINATE_MESSAGE_TAG; // Indica que é uma resposta de finalização
  snprintf(message.key, MAX_KEY_VALUE_LENGTH, "");
  snprintf(message.value, MAX_KEY_VALUE_LENGTH, "");
  return message;
}

// Função que lê as operações de um arquivo e as armazena em um vetor de operações
void parse_operations(const char *filename, operation_t **operations, int *operation_count) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Unable to open file");
    exit(EXIT_FAILURE);
  }

  char line[256];
  int count = 0;
  while (fgets(line, sizeof(line), file)) {
    count++;
  }

  fseek(file, 0, SEEK_SET);

  *operations = (operation_t *)malloc(count * sizeof(operation_t));
  *operation_count = count;
  int index = 0;

  while (fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = 0; // Remove newline character

    if (strncmp(line, "READ", 4) == 0) {
      sscanf(line, "%s %s", (*operations)[index].operation, (*operations)[index].key);
    } else if (strncmp(line, "WRITE", 5) == 0) {
      sscanf(line, "%s %s %s", (*operations)[index].operation, (*operations)[index].key, (*operations)[index].value);
    }
    index++;
  }

  fclose(file);
}