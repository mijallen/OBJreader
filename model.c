#include "model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>

#include "hashmap.h"
#include "vector.h"
#include "3dMath.h"



/* object fields */



struct model {
  float* vertex;
  float* normal;
  float* texcoord;
  float* color;

  unsigned int* triangle;
  unsigned int* groupTag;

  size_t vertexCount;
  size_t triangleCount;
};



/* helper functions */



void vector_getv(float* vOut, vector_t* V, size_t index) {
  vOut[0] = vector_getf(V, index + 0);
  vOut[1] = vector_getf(V, index + 1);
  vOut[2] = vector_getf(V, index + 2);
}



void vector_pushv(vector_t* V, float* vec) {
  vector_pushf(V, vec[0]);
  vector_pushf(V, vec[1]);
  vector_pushf(V, vec[2]);
}



void load_dataGather(FILE* infile, vector_t* position_data, vector_t* texcoord_data, vector_t* normal_data) {
  vector_t *tokens;
  char line[1024];

  tokens = vector_create();

  rewind(infile);

  while (fgets(line, 1023, infile) != NULL) {
    vector_getTokens(tokens, line, " \t\r\n");

    if (vector_size(tokens) == 0) {
      vector_deepClear(tokens);
      continue;
    }

    if (strcmp((char*)vector_get(tokens, 0), "v") == 0) {
      vector_pushf( position_data, strtod( (char*)vector_get(tokens, 1), NULL ) );
      vector_pushf( position_data, strtod( (char*)vector_get(tokens, 2), NULL ) );
      vector_pushf( position_data, strtod( (char*)vector_get(tokens, 3), NULL ) );
    }
    else if (strcmp((char*)vector_get(tokens, 0), "vt") == 0) {
      vector_pushf( texcoord_data, strtod( (char*)vector_get(tokens, 1), NULL ) );
      vector_pushf( texcoord_data, strtod( (char*)vector_get(tokens, 2), NULL ) );
    }
    else if (strcmp((char*)vector_get(tokens, 0), "vn") == 0) {
      vector_pushf( normal_data, strtod( (char*)vector_get(tokens, 1), NULL ) );
      vector_pushf( normal_data, strtod( (char*)vector_get(tokens, 2), NULL ) );
      vector_pushf( normal_data, strtod( (char*)vector_get(tokens, 3), NULL ) );
    }

    vector_deepClear(tokens);
  }

  vector_destroy(tokens);
}



/* constructor */



model_t* model_load(const char* filePath) {
  model_t *out;

  vector_t *position_data, *texcoord_data, *normal_data;
  vector_t *position, *texcoord, *normal;

  vector_t *triangle, *groupTag;
  vector_t *tokens, *face_elements;

  unsigned int iter, vertex_iter, collection[3];
  unsigned int faceTag = 0, vertex_count = 0;
  unsigned int position_index, texcoord_index, normal_index;

  float position_temp[3], texcoord_temp[3], normal_temp[3];

  FILE* infile;
  char line[1024];
  char *vertex_str, *copy;

  mapnode_t* result;
  hashmap_t* vertices = hashmap_create();

  out = (model_t*)calloc(1, sizeof(model_t));

  position_data = vector_create();
  texcoord_data = vector_create();
  normal_data = vector_create();

  position = vector_create();
  texcoord = vector_create();
  normal = vector_create();

  triangle = vector_create();
  groupTag = vector_create();

  tokens = vector_create();
  face_elements = vector_create();

  infile = fopen(filePath, "r");

  load_dataGather(infile, position_data, texcoord_data, normal_data);
  rewind(infile);

  while (fgets(line, 1023, infile) != NULL) {
    vector_getTokens(tokens, line, " \t\r\n");

    if (vector_size(tokens) == 0) {
      vector_deepClear(tokens);
      continue;
    }

    if (strcmp((char*)vector_get(tokens, 0), "f") == 0) {
      for (iter = 2; iter < vector_size(tokens) - 1; ++iter) {

        collection[0] = 1;
        collection[1] = iter;
        collection[2] = iter + 1;

        for (vertex_iter = 0; vertex_iter < 3; ++vertex_iter) {

          vertex_str = (char*)vector_get(tokens, collection[vertex_iter]);
          copy = (char*)calloc(strlen(vertex_str) + 1, sizeof(char));
          sprintf(copy, "%s", vertex_str);

          result = hashmap_search(vertices, copy);
          if (result == NULL) {
            printf("inserted vertex data '%s' with value %u\n", copy, vertex_count);
            hashmap_insert(vertices, copy, vertex_count);

            vector_getTokens(face_elements, (char*)vector_get(tokens, collection[vertex_iter]), "/");

            position_index = strtol( (char*)vector_get(face_elements, 0), NULL, 10 ) - 1;
            if (vector_size(face_elements) > 1)
              texcoord_index = strtol( (char*)vector_get(face_elements, 1), NULL, 10 ) - 1;
            if (vector_size(face_elements) > 2)
              normal_index = strtol( (char*)vector_get(face_elements, 2), NULL, 10 ) - 1;

            vector_deepClear(face_elements);

            vector_getv(position_temp, position_data, 3 * position_index);
            if (vector_size(face_elements) > 1)
              vector_getv(texcoord_temp, texcoord_data, 3 * texcoord_index);
            if (vector_size(face_elements) > 2)
              vector_getv(normal_temp, normal_data, 3 * normal_index);

            vector_pushv(position, position_temp);
            if (vector_size(face_elements) > 1)
              vector_pushv(texcoord, texcoord_temp);
            if (vector_size(face_elements) > 2)
              vector_pushv(normal, normal_temp);

            vector_pushi(triangle, vertex_count);

            ++vertex_count;
          } else {
            vector_pushi(triangle, mapnode_value(result));

            free(copy);
          }

        }

        vector_pushi(groupTag, faceTag);
      }

      ++faceTag;
    }

    vector_deepClear(tokens);
  }

  fclose(infile);

  hashmap_deepClear(vertices);
  hashmap_destroy(vertices);

  out->vertexCount = vector_size(position) / 3;
  out->triangleCount = vector_size(triangle) / 3;

  out->vertex = vector_arrayf(position);
  out->texcoord = (vector_size(texcoord) > 0) ? vector_arrayf(texcoord) : NULL;
  out->normal = (vector_size(normal) > 0) ? vector_arrayf(normal) : NULL;

  out->triangle = (unsigned int*)vector_arrayi(triangle);
  out->groupTag = (unsigned int*)vector_arrayi(groupTag);

  vector_destroy(position_data);
  vector_destroy(texcoord_data);
  vector_destroy(normal_data);

  vector_destroy(position);
  vector_destroy(texcoord);
  vector_destroy(normal);

  vector_destroy(triangle);
  vector_destroy(groupTag);

  vector_destroy(tokens);
  vector_destroy(face_elements);

  return out;
}



/* accessor functions (getter) */



size_t model_vertexCount(model_t* M) {
  return M->vertexCount;
}

size_t model_triangleCount(model_t* M) {
  return M->triangleCount;
}



float model_calculateRadius(model_t* M) {
  size_t iter;
  float len, radius = 0.f;

  for (iter = 0; iter < M->vertexCount; ++iter) {
    len = magnitude( &(M->vertex[3 * iter]) );
    if (radius < len) radius = len;
  }

  return radius;
}

void model_drawGL(model_t* M) {
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, M->vertex);

  if (M->normal != NULL) {
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, M->normal);
  }

  if (M->texcoord != NULL) {
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, M->texcoord);
  }

  if (M->color != NULL) {
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(3, GL_FLOAT, 0, M->color);
  }

  glDrawElements(GL_TRIANGLES, 3 * M->triangleCount, GL_UNSIGNED_INT, M->triangle);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}



/* modifier functions (setter) */



void model_calculateNormals(model_t* M) {
  size_t iter, vertexA, vertexB, vertexC;
  unsigned int faceTag, prevTag;
  float edgeAB[3], edgeAC[3], cross[3];

  float* normal = (float*)calloc(M->vertexCount, 3 * sizeof(float));
  if (M->normal != NULL) free(M->normal);

  for (iter = 0; iter < M->triangleCount; ++iter) {
    vertexA = 3 * M->triangle[3 * iter + 0];
    vertexB = 3 * M->triangle[3 * iter + 1];
    vertexC = 3 * M->triangle[3 * iter + 2];

    faceTag = M->groupTag[iter];
    prevTag = (iter != 0) ? M->groupTag[iter - 1] : faceTag;

    subtract_vec(edgeAB, &(M->vertex[vertexB]), &(M->vertex[vertexA]));
    subtract_vec(edgeAC, &(M->vertex[vertexC]), &(M->vertex[vertexA]));
    cross_product(cross, edgeAB, edgeAC);
    normalize(cross);

    if (faceTag != prevTag) {
      add_vec(&(normal[vertexA]), &(normal[vertexA]), cross);
      add_vec(&(normal[vertexB]), &(normal[vertexB]), cross);
      add_vec(&(normal[vertexC]), &(normal[vertexC]), cross);
    } else {
      if (magnitude( &(normal[vertexA]) ) < 0.001f)
        add_vec( &(normal[vertexA]), &(normal[vertexA]), cross );
      if (magnitude( &(normal[vertexB]) ) < 0.001f)
        add_vec( &(normal[vertexB]), &(normal[vertexB]), cross );
      if (magnitude( &(normal[vertexC]) ) < 0.001f)
        add_vec( &(normal[vertexC]), &(normal[vertexC]), cross );
    }
  }

  for (iter = 0; iter < M->vertexCount; ++iter) normalize( &(normal[3 * iter]) );

  M->normal = normal;
}

void model_center(model_t* M) { /* NOTE: can change output of model_radius */
  size_t iter, vertexA, vertexB, vertexC;
  float xSum = 0.f, ySum = 0.f, zSum = 0.f, scale = 0.f;
  float tempX, tempY, tempZ, weight;
  float edgeAB[3], edgeAC[3], cross[3];

  for (iter = 0; iter < M->triangleCount; ++iter) {
    vertexA = 3 * M->triangle[3 * iter + 0];
    vertexB = 3 * M->triangle[3 * iter + 1];
    vertexC = 3 * M->triangle[3 * iter + 2];

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

  for (iter = 0; iter < M->vertexCount; ++iter) {
    M->vertex[3 * iter + 0] -= xSum * scale;
    M->vertex[3 * iter + 1] -= ySum * scale;
    M->vertex[3 * iter + 2] -= zSum * scale;
  }
}

void model_randomColors(model_t* M) {
  size_t iter;

  float* color = (float*)calloc(M->vertexCount, 3 * sizeof(float));
  if (M->color != NULL) free(M->color);

  for (iter = 0; iter < M->vertexCount; ++iter) {
    color[3 * iter + 0] = rand_fract();
    color[3 * iter + 1] = rand_fract();
    color[3 * iter + 2] = rand_fract();
  }

  M->color = color;
}



/* destructor */



void model_destroy(model_t* M) {
  free(M->vertex);
  if (M->normal != NULL) free(M->normal);
  if (M->texcoord != NULL) free(M->texcoord);
  if (M->color != NULL) free(M->color);

  free(M->triangle);
  free(M->groupTag);

  free(M);
}
