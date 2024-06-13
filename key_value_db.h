#ifndef KEY_VALUE_DB_H
#define KEY_VALUE_DB_H

#include <stddef.h>

// Define o tamanho inicial da tabela hash
#define TABLE_SIZE 100

// Define a estrutura para um par chave-valor
typedef struct KeyValue {
  char *key;
  char *value;
  struct KeyValue *next;
} KeyValue;

// Define a estrutura para o banco de dados de chave-valor
typedef struct {
  KeyValue **table;
} KeyValueDB;

// Inicializa o banco de dados de chave-valor
void initDB(KeyValueDB *db);

// Insere ou atualiza um par chave-valor no banco de dados
void upsertEntry(KeyValueDB *db, const char *key, const char *value);

// Obtém o valor para uma dada chave no banco de dados
const char *getValue(KeyValueDB *db, const char *key);

// Libera todos os recursos associados ao banco de dados
void freeDB(KeyValueDB *db);

#endif // KEY_VALUE_DB_H
