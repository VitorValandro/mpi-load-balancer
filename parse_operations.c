#include "common.h"
#include <stdio.h>
#include <stdlib.h>

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