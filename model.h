#ifndef _MODEL_H
#define _MODEL_H

struct model;
typedef struct model model_t;

/* constructor */

model_t* model_load(const char* filePath);

/* accessor functions (getter) */

unsigned int model_vertexCount(model_t*);
unsigned int model_indexCount(model_t*);

float model_calculateRadius(model_t*);
void model_drawGL(model_t*);

/* modifier functions (setter) */

void model_calculateNormals(model_t*);
void model_center(model_t*); /* NOTE: can change output of model_radius */
void model_randomColors(model_t*);

/* destructor */

void model_destroy(model_t*);

#endif
