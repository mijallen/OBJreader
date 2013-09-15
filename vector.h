#ifndef _VECTOR_H
#define _VECTOR_H

struct vector;
typedef struct vector vector_t;

/* constructor */

vector_t* vector_create();

/* accessor functions (getter) */

void* vector_get(vector_t*, size_t index);
int vector_geti(vector_t*, size_t index);
float vector_getf(vector_t*, size_t index);

void** vector_array(vector_t*);
int* vector_arrayi(vector_t*);
float* vector_arrayf(vector_t*);

size_t vector_size(vector_t*);

/* modifier functions (setter) */

void vector_push(vector_t*, void* data);
void vector_pushi(vector_t*, int data);
void vector_pushf(vector_t*, float data);

void vector_clear(vector_t*);
void vector_deepClear(vector_t*);

/* dunno yet */

void vector_getTokens(vector_t*, const char* str, const char* delimiters);

/* destructor */

void vector_destroy(vector_t*);

#endif
