#include "common.h"

void client(int rank, int world_size, char *processor_name) {
  printf("Client Rank %d, processor %s\n", rank, processor_name);
  int load_balancer_rank = LOAD_BALANCER_RANK;

  // TODO: enviar requisição para o load balancer
  // TODO: receber resposta da replica
}
