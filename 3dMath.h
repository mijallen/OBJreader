#ifndef _3DMATH_H
#define _3DMATH_H

#include <math.h>

void cross_product(float* vOut, float* vA, float* vB) {
  vOut[0] = vA[1] * vB[2] - vA[2] * vB[1];
  vOut[1] = vA[2] * vB[0] - vA[0] * vB[2];
  vOut[2] = vA[0] * vB[1] - vA[1] * vB[0];
  return; /* output found in vOut */
}

void add_vec(float* vOut, float* vA, float* vB) {
  vOut[0] = vA[0] + vB[0];
  vOut[1] = vA[1] + vB[1];
  vOut[2] = vA[2] + vB[2];
  return; /* output found in vOut */
}

void subtract_vec(float* vOut, float* vA, float* vB) {
  vOut[0] = vA[0] - vB[0];
  vOut[1] = vA[1] - vB[1];
  vOut[2] = vA[2] - vB[2];
  return; /* output found in vOut */
}

void normalize(float* v) {
  float invLen = 1.f / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  v[0] *= invLen;
  v[1] *= invLen;
  v[2] *= invLen;
  return; /* output found in v */
}

float magnitude(float* v) {
  return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

int equal_vec(float* vA, float* vB, float tolerance) {
  if (fabs(vA[0] - vB[0]) > tolerance) return 0;
  if (fabs(vA[1] - vB[1]) > tolerance) return 0;
  if (fabs(vA[2] - vB[2]) > tolerance) return 0;
  return 1;
}

float rand_fract() {
  return ( (float)rand() / (float)RAND_MAX );
}

#endif
