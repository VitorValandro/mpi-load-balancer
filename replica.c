#include "common.h"
#include "key_value_db.h"
#include <stdio.h>

void replica(int rank, int world_size, char *processor_name) {
  printf("Replica Rank %d, processor %s\n", rank, processor_name);
  KeyValueDB db;
  initDB(&db);

  // TODO: receber mensagens do load balancer
  // TODO: responder mensagens para a replica

  freeDB(&db);
}