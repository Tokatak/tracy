#include "tracer.h"

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
