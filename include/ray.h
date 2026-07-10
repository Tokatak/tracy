#ifndef RAY_H
#define RAY_H

#include "v3.h"
#include <stddef.h>

typedef struct{
  V3 position;
  float radius;
  V3 color;
  float specular;
  float reflective;
} Sphere;

typedef struct{
  float t1;
  float t2;
  Sphere* sphere;
} RaySphereIntersection;

V3 ReflectRay(V3 N, V3 R);
RaySphereIntersection intersectRaySphere( V3 O, V3 D, Sphere* sphere);
RaySphereIntersection intersectRaySphereClosest(V3 O, V3 D, float t_min, float t_max, Sphere* spheres, int sphereCount);

#endif
