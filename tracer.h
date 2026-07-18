#ifndef PPM_H
#define PPM_H

#include <stdio.h>

void save(unsigned char* buffer, int widthPixel, int heightPixel, char* fileName){
  FILE *fptr;
  fptr = fopen(fileName, "w");
  
  fprintf(fptr, "P3\n");
  fprintf(fptr, "%d %d\n", widthPixel, heightPixel);
  fprintf(fptr, "255\n");

  for( int j=0; j < heightPixel; j++){
    for ( int i=0; i< widthPixel; i++){
      int pixel = (i + widthPixel * j) *3;
      fprintf(fptr, "%d %d %d ", buffer[pixel +0], buffer[pixel+1], buffer[pixel+2]);
    }
  }
}

#endif

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


#ifndef UTILITY_H
#define UTILITY_H

#define ARRAY_SIZE(arr)(sizeof(arr)/sizeof((arr)[0]))
#define BIG_NUMBER 9e10
#define EPSILON 0.001f

#endif


#ifndef RAY_H
#define RAY_H

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


V3 ReflectRay(V3 N, V3 R){
  V3 result = {0};
  float nDotl = v3_dot(R,N);
  result.x = 2*N.x*nDotl-R.x;
  result.y = 2*N.y*nDotl-R.y;
  result.z = 2*N.z*nDotl-R.z;
  return result;
}


RaySphereIntersection intersectRaySphere( V3 O, V3 D, Sphere* sphere){
  float r = sphere->radius;
  RaySphereIntersection result = {0};
  V3 sphereCenter = sphere->position;
  
  V3 CO = {O.x - sphereCenter.x, O.y - sphereCenter.y, O.z - sphereCenter.z };
  float a = v3_dot(D, D);
  float b = 2*v3_dot(CO, D);
  float c = v3_dot(CO, CO) - r*r;

  float discriminant = b*b - 4*a*c;
  if( discriminant < 0 )
    {
      return result;
    }

  if( discriminant == 0 ){
    result.t1 = (-b + sqrtf(discriminant)) / (2*a);
    result.t2 = result.t1;
    return result;
  }

  if( discriminant > 0 ){
    float sqrtdis = sqrtf(discriminant);
    result.t1 = (-b + sqrtdis) / (2*a);
    result.t2 = (-b - sqrtdis) / (2*a);
    return result;
  }

  return result;
}

RaySphereIntersection intersectRaySphereClosest(V3 O, V3 D,  float t_min, float t_max, Sphere* spheres, int sphereCount){
  RaySphereIntersection  result = {0};

  float closest_t = BIG_NUMBER;
  Sphere* closest_sphere = NULL;

  RaySphereIntersection hit;
  for ( int i=0; i< sphereCount; i++){
    Sphere* current = spheres+i;
    hit = intersectRaySphere( O, D, current );

    if( hit.t1 > t_min && hit.t1 < t_max && hit.t1 < closest_t){
      closest_t = hit.t1;
      closest_sphere = current;
    }

    if( hit.t2 > t_min && hit.t2 < t_max && hit.t2 < closest_t){
      closest_t = hit.t2;
      closest_sphere = current;
    } 
  }

  result.t1 = closest_t;
  result.sphere = closest_sphere;
  return result;
}

#endif


#ifndef TRACER_H
#define TRACER_H

/* static const V3 DEFAULT_COLOR = {0,0,0}; */
#define DEFAULT_COLOR ((V3){0.0f,0.0f,0.0f})

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


// centered zero
// todo:  needs a better name
typedef struct{
  int top;
  int bot;
  int left;
  int right;
} Region;

float ComputeLighting(V3 P, V3 N, V3 View, float s,
		      Sphere* spheres, int sphereCount,
		      Light* lights, int lightCount){
  float intensity = 0.0;
  V3 L;
  V3 Reflection;

  for( int i =0; i< lightCount; i++){
    Light* l = lights+i;

    if (l->type == LIGHT_AMBIENT){
      intensity += l->intensity;
      continue;
    }

    float t_max;
    if ( l->type == LIGHT_POINT ){
      L = v3_sub(l->position, P);
      t_max = 1;
    } else { // DIRECTIONAL
      L = l->position;
      t_max = BIG_NUMBER;
    }

    RaySphereIntersection intersection = intersectRaySphereClosest(P, L, EPSILON, t_max, spheres, sphereCount);
    if( intersection.sphere != NULL ){
      continue;
    }

    
    // DIFFUSE
    float nDotl = v3_dot( N, L);
    if ( nDotl > 0 ){
      intensity += l->intensity * nDotl / (v3_len(N) * v3_len(L)) ;
    }

    // SPECULAR
    if ( s != -1){

      Reflection = ReflectRay(N,L);
      
      float rDotV = v3_dot( Reflection, View);
      if (rDotV >0){
	intensity += l->intensity * pow( rDotV / (v3_len(Reflection) * v3_len(View)), s );
      }
    }
    
  }
  return intensity;
}

// todo: scene description
 V3 traceRay( V3 O, V3 D, float t_min, float t_max, int recursion_depth,
	     Sphere* spheres, int sphereCount,
	     Light* lights, int lightCount){
  float closest_t = BIG_NUMBER;
  Sphere *closestSphere = NULL;
  
  RaySphereIntersection intersection = intersectRaySphereClosest(O, D, t_min, t_max, spheres, sphereCount);
  closestSphere = intersection.sphere;
  closest_t = intersection.t1;


  if( closestSphere == NULL ){
    return DEFAULT_COLOR;
  }
  
  /* // no light */
  /* if(closestSphere!=NULL){ */
  /*   return closestSphere->color; */
  /* } */
  
  // Lit
  V3 P;
  P.x = O.x + closest_t * D.x;
  P.y = O.y + closest_t * D.y;
  P.z = O.z + closest_t * D.z;

  V3 N;
  N.x = P.x - closestSphere->position.x;
  N.y = P.y - closestSphere->position.y;
  N.z = P.z - closestSphere->position.z;;

  V3 local_color = closestSphere->color;

  V3 v = v3_negate(D);
  // v from object to camera = -D from camera to object
  float light = ComputeLighting(P,N,v,closestSphere->specular, spheres, sphereCount, lights, lightCount);

  local_color.x *= light;
  local_color.y *= light;
  local_color.z *= light;

  float reflective = closestSphere->reflective;
  if ( recursion_depth <= 0 || reflective <=0 ){
    return local_color;
  }

  // -D
  V3 R = ReflectRay(N,v);
  V3 reflected_color = traceRay(P, R, EPSILON, BIG_NUMBER, recursion_depth-1, spheres, sphereCount, lights, lightCount);
  V3 result;
  result.x = local_color.x*(1-reflective) + reflected_color.x*reflective;
  result.y = local_color.y*(1-reflective) + reflected_color.y*reflective;
  result.z = local_color.z*(1-reflective) + reflected_color.z*reflective;
  
  return result;
}

void setPixelTexture(float x, float y, V3 color, Buffer *buffer) {
  int byteOffset = (x + buffer->width * y) * 3;
  // potential branchless operation
  // clamp [0, 255]
  buffer->start[byteOffset + 0] =
      (unsigned char)fmaxf(0.0, fminf(color.x, 255.0));
  buffer->start[byteOffset + 1] =
      (unsigned char)fmaxf(0.0, fminf(color.y, 255.0));
  buffer->start[byteOffset + 2] =
      (unsigned char)fmaxf(0.0, fminf(color.z, 255.0));
}


void setPixelCanvas(float x, float y, V3 color, Buffer *buffer) {
  // in x - in Canvas space - w/2 to w/2
  // out x - texture coords  0 to w

  // in y - in Canvas space  [h/2 (top) , -h/2(bot)]
  // by - HEIGHT/2   [ 0 , -h]
  // and *-1 [0, h] according to texture space
  setPixelTexture(x + (buffer->width) / 2, -(y - (buffer->height) / 2), color,
                  buffer);
}


void fillRegion( V3 origin, Region region, V3 viewportSize,float projectionPlane,
		 Buffer buffer,
		 float t_min, float t_max, int recursion_depth,
		 Sphere* spheres, int sphereCount,
		 Light* lights, int lightCount){

  int topEdge = region.top;
  int bottomEdge = region.bot;

  int leftEdge = region.left;
  int righEdge = region.right;

  V3 direction = {0};
  V3 color;

  int width = buffer.width;
  int height = buffer.height;

  unsigned char* bufferStart = buffer.start;
  
  for (int y = topEdge; y > bottomEdge; y--) {
    for (int x = leftEdge; x < righEdge; x++) {
      /* direction = canvasToViewport(x, y, buffer.width, buffer.height, */
      /*                              viewportSize, projectionPlane); */

      // canvasToViewport
      direction.x = x * viewportSize.x / width;
      direction.y = y * viewportSize.y / height;
      direction.z = projectionPlane;

      
      color = traceRay(origin, direction, 1, BIG_NUMBER, recursion_depth,
		       spheres,  sphereCount,
		       lights, lightCount);

      /* setPixelCanvas */
      // in x - in Canvas space - w/2 to w/2
      // out x - texture coords  0 to w
      // in y - in Canvas space  [h/2 (top) , -h/2(bot)]
      // by - HEIGHT/2   [ 0 , -h]
      // and *-1 [0, h] according to texture space


      // todo: clenup this
      int columnOffsetPx = x + width / 2;
      int rowOffsetPx =  -(y - height / 2);
      
      /* /\* setPixelTexture *\/ */
      int byteOffset = (width * rowOffsetPx + columnOffsetPx) * 3;
      /* // potential branchless operation */
      /* // clamp [0, 255] */
      bufferStart[byteOffset + 0] =
	(unsigned char)fmaxf(0.0, fminf(color.x, 255.0));
      bufferStart[byteOffset + 1] =
	(unsigned char)fmaxf(0.0, fminf(color.y, 255.0));
      bufferStart[byteOffset + 2] =
	(unsigned char)fmaxf(0.0, fminf(color.z, 255.0));
      
    }
  }
}



#endif
