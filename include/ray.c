#include "ray.h"

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

  float closest_t = INFINITY;
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

