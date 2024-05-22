#include "keyval_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


void initializeTable(struct KeyValueTable *kt)
{
    kt->size = 0;
    kt->capacity = 20;
    kt->table = malloc(20 * sizeof(KeyValue *));
    if (kt->table == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 20; i++) {
        kt->table[i] = NULL;
    }
}

int hashCode(const char *key)
{
    int hash = 0;
    int len = strlen(key);
    for (int i = 0; i < len; i++)
    {
        hash += key[i];
    }
    return hash;
}

void insert(struct KeyValueTable *kt, const char *key, const char *value)
{
    if ((double)kt->size / kt->capacity > LOAD_FACTOR_THRESHOLD)
    {
        resize(kt);
    }
    
    // Check if the key already exists
    for (int i = 0; i < kt->size; i++)
    {
        if (strcmp(kt->table[i]->key, key) == 0)
        {
            deleteKey(kt, key);
        }
    }

    struct KeyValue *kv = malloc(sizeof(struct KeyValue));
    if (kv == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    
    kv->key = strdup(key);
    kv->value = strdup(value);

    if (kv->key == NULL || kv->value == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    kt->table[kt->size++] = kv;
}


void deleteKey(KeyValueTable *kt, const char *key) {
    for (int i = 0; i < kt->size; i++) {
        if (strcmp(kt->table[i]->key, key) == 0) {
            // Free the memory allocated for key and value
            free(kt->table[i]->key);
            free(kt->table[i]->value);
            free(kt->table[i]);

            // Shift elements to fill the gap
            for (int j = i; j < kt->size - 1; j++) {
                kt->table[j] = kt->table[j + 1];
            }

            // Decrement the size
            kt->size--;
            return;
        }
    }

    printf("Key '%s' not found\n", key);
}

void resize(struct KeyValueTable *kt)
{
    int newCapacity = kt->capacity * 2;
    struct KeyValue **newTable = (struct KeyValue **)malloc(newCapacity * sizeof(struct KeyValue *));
    if (newTable == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < newCapacity; i++)
    {
        newTable[i] = NULL;
    }

    // Rehash all existing elements into the new table
    for (int i = 0; i < kt->capacity; i++)
    {
        if (kt->table[i] != NULL)
        {
            int hashIndex = hashCode(kt->table[i]->key) % newCapacity;
            while (newTable[hashIndex] != NULL)
            {
                hashIndex = (hashIndex + 1) % newCapacity;
            }
            newTable[hashIndex] = kt->table[i];
        }
    }

    // Free memory of old table
    free(kt->table);

    // Assign the new table to kt->table
    kt->table = newTable;
    kt->capacity = newCapacity;
}

// bool areEqual(const char *str1, const char *str2) {
//     printf("he\n");
//     // Iterate over each character of the strings
//     while (*str1 != '\0' && *str2 != '\0') {
//         // If the characters are not equal, return false
//         if (*str1 != *str2) {
//             return false;
//         }
//         // Move to the next character
//         str1++;
//         str2++;
//     }
//     // If both strings have reached the end simultaneously, they are equal
//     return (*str1 == '\0' && *str2 == '\0');
// }

char *getValue(struct KeyValueTable *kt, const char *key)
{
    for (int i = 0; i < kt->size; i++)
    {
        //bool i = areEqual(kt->table[i]->key, key); 
        //printf("%d\n" ,i );
        if (strcmp(kt->table[i]->key, key) == 0)
        {
            return kt->table[i]->value;
        }
    }
    return NULL;
}

void freeTable(struct KeyValueTable *kt)
{
    for (int i = 0; i < kt->size; i++)
    {
        free(kt->table[i]->key);
        free(kt->table[i]->value);
        free(kt->table[i]);
    }
}

// int main() {
//     struct KeyValueTable kt;
//     initializeTable(&kt);

//     insert(&kt, "name", "Alice");
//     insert(&kt, "age", "30");
//     insert(&kt, "city", "Wonderland");
//     insert(&kt, "city", "Wonder");

//     printf("Name: %s\n", getValue(&kt, "name"));
//     printf("Age: %s\n", getValue(&kt, "age"));
//     printf("City: %s\n", getValue(&kt, "city"));

//     freeTable(&kt);

//     return 0;
// }
