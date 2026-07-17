#include "tracer.h"


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

      
      color = traceRay(origin, direction, 1, INFINITY, recursion_depth,
		       spheres,  sphereCount,
		       lights, lightCount);

      /* setPixelCanvas */
      // in x - in Canvas space - w/2 to w/2
      // out x - texture coords  0 to w
      // in y - in Canvas space  [h/2 (top) , -h/2(bot)]
      // by - HEIGHT/2   [ 0 , -h]
      // and *-1 [0, h] according to texture space


      // clenup this
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

V3 traceRay( V3 O, V3 D, float t_min, float t_max, int recursion_depth,
	     Sphere* spheres, int sphereCount,
	     Light* lights, int lightCount){
  float closest_t = INFINITY;
  Sphere *closestSphere = NULL;
  
  RaySphereIntersection intersection = intersectRaySphereClosest(O, D, t_min, t_max, spheres, sphereCount);
  closestSphere = intersection.sphere;
  closest_t = intersection.t1;


  if( closestSphere == NULL ){
    return DEFAULT_COLOR;
  }
  
  // no light 
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
  V3 reflected_color = traceRay(P, R, EPSILON, INFINITY, recursion_depth-1, spheres, sphereCount, lights, lightCount);
  V3 result;
  result.x = local_color.x*(1-reflective) + reflected_color.x*reflective;
  result.y = local_color.y*(1-reflective) + reflected_color.y*reflective;
  result.z = local_color.z*(1-reflective) + reflected_color.z*reflective;
  
  return result;
}

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
      t_max = INFINITY;
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
