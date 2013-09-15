#include "model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>

#include "vector.h"
#include "3dMath.h"



/* object fields */



struct model {
  float* vertex;
  float* normal;
  float* color;
  int* index;

  size_t vertexCount;
  size_t indexCount;
};



/* constructor */



model_t* model_load(const char* filePath) {
  model_t *out;
  vector_t *vertex, *index;
  vector_t *tokens, *face_elements;
  size_t iter;

  FILE* infile;
  char line[1024];

  out = (model_t*)calloc(1, sizeof(model_t));

  vertex = vector_create();
  index = vector_create();
  tokens = vector_create();
  face_elements = vector_create();

  infile = fopen(filePath, "r");

  while (fgets(line, 1023, infile) != NULL) {
    vector_getTokens(tokens, line, " ");

    if (vector_size(tokens) > 0) {
      if (strcmp((char*)vector_get(tokens, 0), "v") == 0) {
        vector_pushf( vertex, strtod( (char*)vector_get(tokens, 1), NULL ) );
        vector_pushf( vertex, strtod( (char*)vector_get(tokens, 2), NULL ) );
        vector_pushf( vertex, strtod( (char*)vector_get(tokens, 3), NULL ) );
      }

      if (strcmp((char*)vector_get(tokens, 0), "f") == 0) {
        for (iter = 2; iter < vector_size(tokens) - 1; ++iter) {
          vector_getTokens(face_elements, (char*)vector_get(tokens, 1), "/");
          vector_pushi( index, strtol( (char*)vector_get(face_elements, 0), NULL, 10) - 1 );
          vector_deepClear(face_elements);

          vector_getTokens(face_elements, (char*)vector_get(tokens, iter), "/");
          vector_pushi( index, strtol( (char*)vector_get(face_elements, 0), NULL, 10) - 1 );
          vector_deepClear(face_elements);

          vector_getTokens(face_elements, (char*)vector_get(tokens, iter + 1), "/");
          vector_pushi( index, strtol( (char*)vector_get(face_elements, 0), NULL, 10) - 1 );
          vector_deepClear(face_elements);
        }
      }
    }

    vector_deepClear(tokens);
  }

  fclose(infile);

  out->vertexCount = vector_size(vertex);
  out->indexCount = vector_size(index);

  out->vertex = vector_arrayf(vertex);
  out->index = vector_arrayi(index);

  vector_destroy(vertex);
  vector_destroy(index);
  vector_destroy(tokens);
  vector_destroy(face_elements);

  return out;
}



/* accessor functions (getter) */



size_t model_vertexCount(model_t* M) {
  return M->vertexCount;
}

size_t model_indexCount(model_t* M) {
  return M->indexCount;
}



float model_calculateRadius(model_t* M) {
  size_t iter;
  float len, radius = 0.f;

  for (iter = 0; iter < M->vertexCount; iter += 3) {
    len = magnitude( &(M->vertex[iter]) );
    if (radius < len) radius = len;
  }

  return radius;
}

void model_drawGL(model_t* M) {
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, M->vertex);

  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, 0, M->normal);

  glEnableClientState(GL_COLOR_ARRAY);
  glColorPointer(3, GL_FLOAT, 0, M->color);

  glDrawElements(GL_TRIANGLES, M->indexCount, GL_UNSIGNED_INT, M->index);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}



/* modifier functions (setter) */



void model_calculateNormals(model_t* M) {
  size_t iter, vertexA, vertexB, vertexC;
  float edgeAB[3], edgeAC[3], cross[3], prev[3];

  float* normal = (float*)calloc(M->vertexCount, sizeof(float));
  if (M->normal != NULL) free(M->normal);
  prev[0] = 0.f; prev[1] = 0.f; prev[2] = 0.f;

  for (iter = 0; iter < M->indexCount; iter += 3) {
    vertexA = 3 * M->index[iter + 0];
    vertexB = 3 * M->index[iter + 1];
    vertexC = 3 * M->index[iter + 2];

    subtract_vec(edgeAB, &(M->vertex[vertexB]), &(M->vertex[vertexA]));
    subtract_vec(edgeAC, &(M->vertex[vertexC]), &(M->vertex[vertexA]));
    cross_product(cross, edgeAB, edgeAC);
    normalize(cross);

    if (!equal_vec(cross, prev, 0.001f)) {
      add_vec(&(normal[vertexA]), &(normal[vertexA]), cross);
      add_vec(&(normal[vertexB]), &(normal[vertexB]), cross);
      add_vec(&(normal[vertexC]), &(normal[vertexC]), cross);
      prev[0] = cross[0]; prev[1] = cross[1]; prev[2] = cross[2];
    } else {
      if (magnitude( &(normal[vertexA]) ) < 0.001f)
        add_vec( &(normal[vertexA]), &(normal[vertexA]), cross );
      if (magnitude( &(normal[vertexB]) ) < 0.001f)
        add_vec( &(normal[vertexB]), &(normal[vertexB]), cross );
      if (magnitude( &(normal[vertexC]) ) < 0.001f)
        add_vec( &(normal[vertexC]), &(normal[vertexC]), cross );
    }
  }

  for (iter = 0; iter < M->vertexCount; iter += 3) normalize( &(normal[iter]) );

  M->normal = normal;
}

void model_center(model_t* M) { /* NOTE: can change output of model_radius */
  size_t iter, vertexA, vertexB, vertexC;
  float xSum = 0.f, ySum = 0.f, zSum = 0.f, scale = 0.f;
  float tempX, tempY, tempZ, weight;
  float edgeAB[3], edgeAC[3], cross[3];

  for (iter = 0; iter < M->indexCount; iter += 3) {
    vertexA = 3 * M->index[iter + 0];
    vertexB = 3 * M->index[iter + 1];
    vertexC = 3 * M->index[iter + 2];

    tempX = M->vertex[vertexA + 0] + M->vertex[vertexB + 0];
    tempX = 0.333f * (tempX + M->vertex[vertexC + 0]);

    tempY = M->vertex[vertexA + 1] + M->vertex[vertexB + 1];
    tempY = 0.333f * (tempY + M->vertex[vertexC + 1]);

    tempZ = M->vertex[vertexA + 2] + M->vertex[vertexB + 2];
    tempZ = 0.333f * (tempZ + M->vertex[vertexC + 2]);

    subtract_vec(edgeAB, &(M->vertex[vertexB]), &(M->vertex[vertexA]));
    subtract_vec(edgeAC, &(M->vertex[vertexC]), &(M->vertex[vertexA]));
    cross_product(cross, edgeAB, edgeAC);
    weight = magnitude(cross);

    xSum += tempX * weight;
    ySum += tempY * weight;
    zSum += tempZ * weight;
    scale += weight;
  }

  scale = 1.f / scale;

  for (iter = 0; iter < M->vertexCount; iter += 3) {
    M->vertex[iter + 0] -= xSum * scale;
    M->vertex[iter + 1] -= ySum * scale;
    M->vertex[iter + 2] -= zSum * scale;
  }
}

void model_randomColors(model_t* M) {
  size_t iter;

  float* color = (float*)calloc(M->vertexCount, sizeof(float));
  if (M->color != NULL) free(M->color);

  for (iter = 0; iter < M->vertexCount; ++iter) {
    color[iter] = rand_fract();
  }

  M->color = color;
}



/* destructor */



void model_destroy(model_t* M) {
  free(M->vertex);
  if (M->normal != NULL) free(M->normal);
  if (M->color != NULL) free(M->color);
  free(M->index);
  free(M);
}
