#ifndef KEYVAL_TABLE_H
#define KEYVAL_TABLE_H

#define LOAD_FACTOR_THRESHOLD 0.7

typedef struct KeyValue {
    char *key;
    char *value;
}KeyValue;

typedef struct KeyValueTable {
    struct KeyValue **table;
    int size;
    int capacity;
} KeyValueTable;

void initializeTable(struct KeyValueTable *kt);
void insert(struct KeyValueTable *kt, const char *key, const char *value);
char *getValue(struct KeyValueTable *kt, const char *key);
void freeTable(struct KeyValueTable *kt);
void resize(struct KeyValueTable *kt);
int hashCode(const char *key);
void deleteKey(KeyValueTable *kt, const char *key);

#endif /* KEYVAL_TABLE_H */
