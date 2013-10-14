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



char* copyStr(char* src) {
  char* out = (char*)calloc( strlen(src) + 1, sizeof(char) );
  sprintf(out, "%s", src);
  return out;
}



unsigned int token_count(char* str) {
  unsigned int iter, size;
  unsigned int count = 0;
  int onToken = 0;
  char temp;

  size = strlen(str);

  for (iter = 0; iter < size; ++iter) {
    temp = str[iter];

    if (temp == '\r' || temp == '\n') break;
    if (temp == ' ' || temp == '\t') {
      onToken = 0;
      continue;
    }

    if (onToken == 0) ++count;
    onToken = 1;
  }

  return count;
}



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



int load_validate(FILE* infile) {
  if (infile == NULL) {
    fprintf(stderr, "model_load (validation): OBJ file not found\n");
    return 0;
  }

  rewind(infile);

  return 1;
}



void load_dataGather(FILE* infile, vector_t* position_data, vector_t* texcoord_data, vector_t* normal_data) {
  char line[1024], id[32];
  float vector[3];

  rewind(infile);

  while (fgets(line, 1023, infile) != NULL) {
    if (token_count(line) == 0) continue;
    sscanf(line, "%s", id);

    if (strcmp(id, "v") == 0) {
      sscanf(line, "%s %g %g %g", id, &(vector[0]), &(vector[1]), &(vector[2]));
      vector_pushv(position_data, vector);
    }
    else if (strcmp(id, "vt") == 0) {
      sscanf(line, "%s %g %g", id, &(vector[0]), &(vector[1]));
      vector_pushf(texcoord_data, vector[0]);
      vector_pushf(texcoord_data, vector[1]);
    }
    else if (strcmp(id, "vn") == 0) {
      sscanf(line, "%s %g %g %g", id, &(vector[0]), &(vector[1]), &(vector[2]));
      vector_pushv(normal_data, vector);
    }

  }

}



char** load_hashVertices(FILE* infile, model_t* M) {
  char line[1024], id[32];
  vector_t *triangle, *groupTag, *tokens;
  unsigned int iter, faceTag = 0, vertexCount = 0;
  char *vertex_str;
  hashmap_t* out;
  mapnode_t* result;
  char** output;

  rewind(infile);

  triangle = vector_create();
  groupTag = vector_create();
  tokens = vector_create();

  out = hashmap_create();

  while (fgets(line, 1023, infile) != NULL) {
    sscanf(line, "%s", id);

    if (strcmp(id, "f") != 0) continue;
    vector_getTokens(tokens, line, " \t\r\n");

    for (iter = 1; iter < vector_size(tokens); ++iter) {
      if (iter > 3) {
        vector_pushi( triangle, vector_geti(triangle, vector_size(triangle) - 3) );
        vector_pushi( triangle, vector_geti(triangle, vector_size(triangle) - 2) );
        vector_pushi(groupTag, faceTag);
      } else if (iter == 3) vector_pushi(groupTag, faceTag);

      vertex_str = (char*)vector_get(tokens, iter);
      result = hashmap_search(out, vertex_str);

      if (result == NULL) {
        hashmap_insert(out, copyStr(vertex_str), faceTag);
        vector_pushi(triangle, vertexCount);
        vertexCount += 1;
      } else {
        vector_pushi(triangle, mapnode_index(result));
      }
    }
    faceTag += 1;

    vector_deepClear(tokens);
  }

  M->triangleCount = vector_size(triangle) / 3;
  M->vertexCount = hashmap_used(out);

  M->triangle = (unsigned int*)vector_arrayi(triangle);
  M->groupTag = (unsigned int*)vector_arrayi(groupTag);

  vector_destroy(tokens);
  vector_destroy(groupTag);
  vector_destroy(triangle);

  output = hashmap_arrayKey(out);
  hashmap_destroy(out);

  return output;
}



void load_constructModel(model_t* M, char** vertex_data, vector_t* position_data, vector_t* texcoord_data, vector_t* normal_data) {
  unsigned int iter, vertex_index, texcoord_index, normal_index;
  vector_t* vertex;
  vector_t* texcoord;
  vector_t* normal;
  float temp[3];

  vertex = vector_create();
  texcoord = vector_create();
  normal = vector_create();

  for (iter = 0; iter < M->vertexCount; ++iter) {
    sscanf(vertex_data[iter], "%u/%u/%u", &vertex_index, &texcoord_index, &normal_index);
    --vertex_index;
    --texcoord_index;
    --normal_index;

    vector_getv(temp, position_data, 3 * vertex_index);
    vector_pushv(vertex, temp);

    if (vector_size(texcoord_data) > 0) {
      temp[0] = vector_getf(texcoord_data, 2 * texcoord_index + 0);
      temp[1] = vector_getf(texcoord_data, 2 * texcoord_index + 1);
      vector_pushf(texcoord, temp[0]);
      vector_pushf(texcoord, temp[1]);
    }

    if (vector_size(normal_data) > 0) {
      vector_getv(temp, normal_data, 3 * normal_index);
      vector_pushv(normal, temp);
    }

    free(vertex_data[iter]);
  }

  M->vertex = vector_arrayf(vertex);
  if (vector_size(texcoord_data) > 0) M->texcoord = vector_arrayf(texcoord);
  if (vector_size(normal_data) > 0) M->normal = vector_arrayf(normal);

  vector_destroy(vertex);
  vector_destroy(texcoord);
  vector_destroy(normal);

  free(vertex_data);
}



/* constructor */



model_t* model_load(const char* filePath) {
  model_t *out;

  vector_t *position_data, *texcoord_data, *normal_data;

  unsigned int iter;

  FILE* infile;
  char** temp;

  infile = fopen(filePath, "r");
  if (!load_validate(infile)) return NULL;

  out = (model_t*)calloc(1, sizeof(model_t));
  temp = load_hashVertices(infile, out);

  position_data = vector_create();
  texcoord_data = vector_create();
  normal_data = vector_create();

  load_dataGather(infile, position_data, texcoord_data, normal_data);
  load_constructModel(out, temp, position_data, texcoord_data, normal_data);

  fclose(infile);

  printf("---model data---\n");

  printf(" -position data-\n");
  for (iter = 0; iter < out->vertexCount; ++iter) {
    printf("  +position: %g %g %g\n", out->vertex[3 * iter + 0],
                                      out->vertex[3 * iter + 1],
                                      out->vertex[3 * iter + 2]);
  }

  if (out->texcoord != NULL) {
    printf(" -texcoord data-\n");
    for (iter = 0; iter < out->vertexCount; ++iter) {
      printf("  +texcoord: %g %g\n", out->texcoord[2 * iter + 0],
                                     out->texcoord[2 * iter + 1]);
    }
  }

  if (out->normal != NULL) {
    printf(" -normal data-\n");
    for (iter = 0; iter < out->vertexCount; ++iter) {
      printf("  +normal: %g %g %g\n", out->normal[3 * iter + 0],
                                      out->normal[3 * iter + 1],
                                      out->normal[3 * iter + 2]);
    }
  }

  printf(" -triangle data-\n");
  for (iter = 0; iter < out->triangleCount; ++iter) {
    printf("  +triangle: %u %u %u\n", out->triangle[3 * iter + 0],
                                      out->triangle[3 * iter + 1],
                                      out->triangle[3 * iter + 2]);
  }

  printf(" -group tag data-\n");
  for (iter = 0; iter < out->triangleCount; ++iter) {
    printf("  -groupTag: %u\n", out->groupTag[iter]);
  }

  vector_destroy(position_data);
  vector_destroy(texcoord_data);
  vector_destroy(normal_data);

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
    //prevTag = (iter != 0) ? M->groupTag[iter - 1] : faceTag;
    prevTag = (iter != 0) ? M->groupTag[iter - 1] : 0xFFFFFFFF;

    subtract_vec(edgeAB, &(M->vertex[vertexB]), &(M->vertex[vertexA]));
    subtract_vec(edgeAC, &(M->vertex[vertexC]), &(M->vertex[vertexA]));
    cross_product(cross, edgeAB, edgeAC);
    normalize(cross);

    if (faceTag != prevTag) {
      add_vec(&(normal[vertexA]), &(normal[vertexA]), cross);
      add_vec(&(normal[vertexB]), &(normal[vertexB]), cross); 
    }
    add_vec(&(normal[vertexC]), &(normal[vertexC]), cross);
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
