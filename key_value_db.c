#include "key_value_db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Função de hash para gerar um índice para uma dada chave
static unsigned int hash(const char *key) {
  unsigned int hash = 0;
  while (*key) {
    hash = (hash << 5) + *key++;
  }
  return hash % TABLE_SIZE;
}

// Inicializa o banco de dados de chave-valor
void initDB(KeyValueDB *db) {
  db->table = (KeyValue **)calloc(TABLE_SIZE, sizeof(KeyValue *));
  if (!db->table) {
    fprintf(stderr, "Falha ao alocar memória para a tabela hash\n");
    exit(EXIT_FAILURE);
  }
}

// Insere ou atualiza um par chave-valor no banco de dados
void upsertEntry(KeyValueDB *db, const char *key, const char *value) {
  unsigned int index = hash(key);
  KeyValue *current = db->table[index];
  KeyValue *prev = NULL;

  // Procura a chave na lista encadeada no índice de hash
  while (current != NULL) {
    if (strcmp(current->key, key) == 0) {
      // Chave encontrada, atualiza o valor
      free(current->value);
      current->value = strdup(value);
      return;
    }
    prev = current;
    current = current->next;
  }

  // Chave não encontrada, cria um novo par chave-valor
  KeyValue *newNode = (KeyValue *)malloc(sizeof(KeyValue));
  if (!newNode) {
    fprintf(stderr, "Falha ao alocar memória\n");
    return;
  }
  newNode->key = strdup(key);
  newNode->value = strdup(value);
  newNode->next = db->table[index];

  // Insere o novo nó no índice de hash
  db->table[index] = newNode;
}

// Obtém o valor para uma dada chave no banco de dados
const char *getValue(KeyValueDB *db, const char *key) {
  unsigned int index = hash(key);
  KeyValue *current = db->table[index];

  // Procura a chave na lista encadeada no índice de hash
  while (current != NULL) {
    if (strcmp(current->key, key) == 0) {
      return current->value;
    }
    current = current->next;
  }

  // Chave não encontrada
  return NULL;
}

// Libera todos os recursos associados ao banco de dados
void freeDB(KeyValueDB *db) {
  for (int i = 0; i < TABLE_SIZE; i++) {
    KeyValue *current = db->table[i];
    KeyValue *next;

    // Percorre a lista encadeada e libera cada nó
    while (current != NULL) {
      next = current->next;
      free(current->key);
      free(current->value);
      free(current);
      current = next;
    }
  }

  free(db->table);
}
