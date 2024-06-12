#include "common.h"
#include "key_value_db.h"
#include <stdio.h>

void replica(int n, char *processor_name)
{
  printf("Replica Rank %d, processor %s\n", n, processor_name);
  KeyValueDB db;
  initDB(&db);

  upsertEntry(&db, "key1", "value1");

  freeDB(&db);
}