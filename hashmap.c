#include <stdlib.h>
#include <string.h>

#include "hashmap.h"

#define START_CAPACITY 4



/* object fields */



struct hashmap {
  unsigned int used, capacity;
  mapnode_t** elements;
};



struct mapnode {
  char* key;
  unsigned int value;
};



/* constructor */



hashmap_t* hashmap_create() {
  hashmap_t* out = (hashmap_t*)calloc(1, sizeof(hashmap_t));

  out->capacity = START_CAPACITY;
  out->elements = (mapnode_t**)calloc(out->capacity, sizeof(mapnode_t*));

  return out;
}



mapnode_t* mapnode_create(char* key, unsigned int value) {
  mapnode_t* out = (mapnode_t*)calloc(1, sizeof(mapnode_t));

  out->key = key;
  out->value = value;

  return out;
}



/* helper functions */



unsigned int hash_string(char* str) { /* djb2 hash algorithm */
  unsigned int hash = 5381;

  while (*str != '\0') {
    hash = 33 * hash + *str;
    ++str;
  }

  return hash;
}



/* accessor functions (getter) */



unsigned int hashmap_used(hashmap_t* H) {
  return H->used;
}



mapnode_t* hashmap_search(hashmap_t* H, char* key) {
  char* node_key;
  unsigned int index = hash_string(key) % H->capacity;

  while (H->elements[index] != NULL) {
    node_key = H->elements[index]->key;
    if (strcmp(node_key, key) == 0) return H->elements[index];
    index = (index + 1) % H->capacity;
  }

  return NULL;
}



unsigned int mapnode_value(mapnode_t* M) {
  return M->value;
}



/* assisting modifier functions */



void hashmap_double(hashmap_t* H) {
  unsigned int iter, size, index;
  mapnode_t** newElements;

  size = H->capacity;

  H->capacity *= 2;
  newElements = (mapnode_t**)calloc(H->capacity, sizeof(mapnode_t*));

  for (iter = 0; iter < size; ++iter) {
    if (H->elements[iter] != NULL) {
      index = hash_string(H->elements[iter]->key) % H->capacity;
      while (newElements[index] != NULL) index = (index + 1) % H->capacity;
      newElements[index] = H->elements[iter];
    }
  }

  free(H->elements);
  H->elements = newElements;
}



/* modifier functions (setter) */



void hashmap_insert(hashmap_t* H, char* key, unsigned int value) {
  unsigned int index;
  char* node_key;
  mapnode_t* temp = mapnode_create(key, value);

  H->used += 1;
  if (4 * H->used > H->capacity) hashmap_double(H);

  index = hash_string(key) % H->capacity;
  while (H->elements[index] != NULL) {
    node_key = H->elements[index]->key;
    if (strcmp(node_key, key) == 0) {
      free(temp);
      return; /* could lead to leaks */
    }
    index = (index + 1) % H->capacity;
  }

  H->elements[index] = temp;
}



void hashmap_deepClear(hashmap_t* H) {
  size_t iter;

  H->used = 0;

  for (iter = 0; iter < H->capacity; ++iter) {
    if (H->elements[iter] != NULL) {
      free(H->elements[iter]->key);
      free(H->elements[iter]);
      H->elements[iter] = NULL;
    }
  }
}



/* destructor */



void hashmap_destroy(hashmap_t* H) {
  unsigned int iter;

  for (iter = 0; iter < H->capacity; ++iter) {
    if (H->elements[iter] != NULL) free(H->elements[iter]);
  }

  free(H->elements);
  free(H);
}
