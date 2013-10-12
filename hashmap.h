#ifndef _HASHMAP_H
#define _HASHMAP_H

struct hashmap;
typedef struct hashmap hashmap_t;

struct mapnode;
typedef struct mapnode mapnode_t;

/* constructor */

hashmap_t* hashmap_create();

/* accessor functions (getter) */

unsigned int hashmap_used(hashmap_t*);
mapnode_t* hashmap_search(hashmap_t*, char* str);
unsigned int mapnode_value(mapnode_t*);
unsigned int mapnode_index(mapnode_t*);
char** hashmap_arrayKey(hashmap_t*);
unsigned int* hashmap_arrayValue(hashmap_t*);

/* modifier functions (setter) */

void hashmap_insert(hashmap_t*, char* str, unsigned int value);
void hashmap_deepClear(hashmap_t*);

/* destructor */

void hashmap_destroy(hashmap_t*);

#endif
