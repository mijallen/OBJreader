#include <stdlib.h>
#include <string.h>

#include "vector.h"

#define START_CAPACITY 16



/* object fields */



struct vector {
  size_t size, capacity;
  void** elements;
};



/* constructor */



vector_t* vector_create() {
  vector_t* V = (vector_t*)calloc(1, sizeof(vector_t));

  V->size = 0;
  V->capacity = START_CAPACITY;
  V->elements = (void**)calloc(V->capacity, sizeof(void*));

  return V;
}



/* accessor fuctions (getter) */



void* vector_get(vector_t* V, size_t index) {
  if (index > V->size - 1) index = V->size - 1;
  return V->elements[index];
}

int vector_geti(vector_t* V, size_t index) {
  void* out = vector_get(V, index);
  return *(int*)&out;
}

float vector_getf(vector_t* V, size_t index) {
  void* out = vector_get(V, index);
  return *(float*)&out;
}



void** vector_array(vector_t* V) {
  void** out = (void**)calloc(V->capacity, sizeof(void*));
  memcpy(out, V->elements, V->capacity * sizeof(void*));
  return out;
}

int* vector_arrayi(vector_t* V) {
  int* out = (int*)calloc(V->capacity, sizeof(int));
  memcpy(out, V->elements, V->capacity * sizeof(int));
  return out;
}

float* vector_arrayf(vector_t* V) {
  float* out = (float*)calloc(V->capacity, sizeof(float));
  memcpy(out, V->elements, V->capacity * sizeof(float));
  return out;
}



size_t vector_size(vector_t* V) {
  return V->size;
}



/* assisting modifier functions */



void vector_double(vector_t* V) {
  void** new_elements = (void**)calloc(2 * V->capacity, sizeof(void*));
  memcpy(new_elements, V->elements, V->capacity * sizeof(void*));
  free(V->elements);
  V->elements = new_elements;
  V->capacity *= 2;
}



/* modifier functions (setter) */



void vector_push(vector_t* V, void* data) {
  if (V->size + 1 > V->capacity) vector_double(V);
  V->elements[V->size] = data;
  ++V->size;
}

void vector_pushi(vector_t* V, int data) {
  vector_push(V, *(void**)&data);
}

void vector_pushf(vector_t* V, float data) {
  vector_push(V, *(void**)&data);
}



void vector_clear(vector_t* V) {
  free(V->elements);

  V->size = 0;
  V->capacity = START_CAPACITY;
  V->elements = (void**)calloc(V->capacity, sizeof(void*));
}

void vector_deepClear(vector_t* V) {
  size_t iter;
  for (iter = 0; iter < V->size; ++iter) free(V->elements[iter]);
  free(V->elements);

  V->size = 0;
  V->capacity = START_CAPACITY;
  V->elements = (void**)calloc(V->capacity, sizeof(void*));
}



/* dunno yet */



void vector_getTokens(vector_t* V, const char* str, const char* delimiters) {
  char *copy, *token, *temp;

  copy = (char*)calloc(strlen(str) + 1, sizeof(char));
  strcpy(copy, str);

  token = strtok(copy, delimiters);

  while (token != NULL) {
    temp = (char*)calloc(strlen(token) + 1, sizeof(char));
    strcpy(temp, token);
    vector_push(V, temp);
    token = strtok(NULL, delimiters);
  }

  free(copy);
}



/* destructor */



void vector_destroy(vector_t* V) {
  free(V->elements);
  free(V);
}
