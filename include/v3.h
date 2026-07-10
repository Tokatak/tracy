#ifndef V3_H
#define V3_H

#include <math.h>

typedef struct{
  float x;
  float y;
  float z;
} V3;

static inline
V3 v3_negate(V3 v){
  v.x=-v.x;
  v.y=-v.y;
  v.z=-v.z;
  return v;
}

static inline
float v3_dot(V3 a, V3 b){
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

static inline
float v3_len(V3 v){
  // TODO: check sqrtf
  return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}

static inline
V3 v3_sub(V3 a, V3 b){
  return (V3){a.x-b.x, a.y - b.y, a.z-b.z};
}

#endif
