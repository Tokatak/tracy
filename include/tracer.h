#ifndef TRACER_H
#define TRACER_H

#include "ray.h"
#include "utility.h"


static const V3 DEFAULT_COLOR = {0,0,0};

typedef enum {LIGHT_AMBIENT, LIGHT_POINT, LIGHT_DIRECTIONAL} LIGHT_TYPE;

typedef struct{
  LIGHT_TYPE type;
  float intensity;
  V3 position;
} Light;

typedef struct{
  int width;
  int height;
  int size;
  unsigned char* start;
} Buffer;


V3 traceRay( V3 O, V3 D, float t_min, float t_max, int recursion_depth,
	     Sphere* spheres, int sphereCount,
	     Light* lights, int lightCount);

float ComputeLighting(V3 P, V3 N, V3 View, float s,
		      Sphere* spheres, int sphereCount,
		      Light* lights, int lightCount);

#endif



