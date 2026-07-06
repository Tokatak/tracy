#include <stdio.h>
#include <math.h>
#include <assert.h> // todo: remove

#define ARRAY_SIZE(arr)(sizeof(arr)/sizeof((arr)[0]))

// 640 480
#define WIDTH 640
#define HEIGHT 480
#define INFINITY 999999

#define DEFAULT_COLOR (V3){0,0,0}
unsigned char buffer[WIDTH*HEIGHT*3] = { 0 };

void save(unsigned char* buffer, int widthPixel, int heightPixel);

typedef struct{
  float x;
  float y;
  float z;
} V3;

float dot(V3 a, V3 b){
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

float len(V3 v){
  return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

typedef struct{
  float x;
  float y;
} V2;  

typedef struct{
  float centerX;
  float centerY;
  float centerZ;
  float radius;
  V3 color;
  float specular;
  float reflective;
} Sphere;

typedef struct{
  int count;
  float t1;
  float t2;
} RaySphereIntersection;

typedef struct{
  Sphere* sphere;
  float distance;
} ClosestIntersection;
// todo: better naming

typedef enum {AMBIENT, POINT, DIRECTIONAL} LIGHT_TYPE;

typedef struct{
  LIGHT_TYPE type;
  float intensity;
  V3 position;
} Light;


V2 viewportSize = {1.0,1.0};
float projectionPlane = 1.0;

Sphere spheres[] = {
  { 0, -1, 3,  1,  {255,   0,  0}, 500, 0.2},
  { 2,  0, 4,  1,  {  0,   0,255}, 500, 0.3},
  {-2,  0, 4,  1,  {  0, 255,  0}, 10 , 0.4},
  {0,-5001,0,5000, {255, 255,  0},1000, 0.5},  
};

Light lights[]= {
  {AMBIENT, 0.2},
  {POINT, 0.6, {2, 1, 0}},
  {DIRECTIONAL, 0.2, {1,4,4}},
};

V3 canvasToViewport(float x, float y){
  V3 result = {0};
  result.x = x * viewportSize.x / WIDTH;
  result.y = y * viewportSize.y / HEIGHT;
  result.z = projectionPlane;
  
  return result;;
}

RaySphereIntersection intersectRaySphere( V3 O, V3 D, Sphere sphere){
  float r = sphere.radius;
  RaySphereIntersection result = {0};
  
  V3 CO = {O.x - sphere.centerX, O.y - sphere.centerY, O.z - sphere.centerZ };
  float a = dot(D, D);
  float b = 2*dot(CO, D);
  float c = dot(CO, CO) - r*r;

  float discriminant = b*b - 4*a*c;
  if( discriminant < 0 )
    {
      result.count = 0;
      return result;
    }

  if( discriminant == 0 ){
    result.count = 1;
    result.t1 = (-b + sqrt(discriminant)) / (2*a);
    return result;
  }

  if( discriminant > 0 ){
    result.count = 2;
    result.t1 = (-b + sqrt(discriminant)) / (2*a);
    result.t2 = (-b - sqrt(discriminant)) / (2*a);
    return result;
  }

  return result;
}

// todo: better naming
ClosestIntersection intersectClosest(V3 O, V3 D,  float t_min, float t_max){
  ClosestIntersection result = {0};

  float closest_t = INFINITY;
  Sphere* closest_sphere = NULL; 

  RaySphereIntersection hit;
  for ( int i=0; i< ARRAY_SIZE(spheres); i++){
    hit = intersectRaySphere( O, D, spheres[i]);

    if ( hit.count == 0){
      continue;
    }

    if ( hit.count == 1){
      if( hit.t1 > t_min && hit.t1 < t_max && hit.t1 < closest_t){
	closest_t = hit.t1;
	closest_sphere = &spheres[i];
      }
      continue;
    }


    if( hit.count ==2 ){
      if( hit.t1 > t_min && hit.t1 < t_max && hit.t1 < closest_t){
	closest_t = hit.t1;
	closest_sphere = &spheres[i];
      }

      if( hit.t2 > t_min && hit.t2 < t_max && hit.t2 < closest_t){
	closest_t = hit.t2;
	closest_sphere = &spheres[i];
      } 
    }
  }

  result.distance = closest_t;
  result.sphere = closest_sphere;
  return result;
}

V3 ReflectRay(V3 N, V3 R){
  V3 result = {0};
  float nDotl = dot(R,N);
  result.x = 2*N.x*nDotl-R.x;
  result.y = 2*N.y*nDotl-R.y;
  result.z = 2*N.z*nDotl-R.z;
  return result;
}

float ComputeLighting(V3 P, V3 N, V3 View, float s){
  float intensity = 0.0;
  V3 L;
  V3 Reflection;

  for( int i =0; i< ARRAY_SIZE(lights); i++){
    Light l = lights[i];

    if (l.type == AMBIENT){
      intensity += l.intensity;
      continue;
    }

    float t_max;
    if ( l.type == POINT ){
      // todo: v3.sub
      L.x = l.position.x - P.x;
      L.y = l.position.y - P.y;
      L.z = l.position.z - P.z;
      t_max = 1;
    } else { // DIRECTIONAL
      // todo: v3.set
      L.x = l.position.x;
      L.y = l.position.y;
      L.z = l.position.z;
      t_max = INFINITY;
    }

    ClosestIntersection intersection = intersectClosest(P, L, 0.001, t_max);
    if( intersection.sphere != NULL ){
      continue;
    }

    
    // DIFFUSE
    float nDotl = dot( N, L);
    if ( nDotl > 0 ){
      intensity += l.intensity * nDotl / (len(N) * len(L)) ;
    }

    // SPECULAR
    if ( s != -1){

      Reflection = ReflectRay(N,L);
      
      float rDotV = dot( Reflection, View);
      if (rDotV >0){
	intensity += l.intensity * pow( rDotV / (len(Reflection) * len(View)), s );
      }
    }
    
  }

  return intensity;
}

V3 traceRay( V3 O, V3 D, float t_min, float t_max, int recursion_depth ){
  V3 color = {0};
  float closest_t = INFINITY;
  Sphere *closestSphere = NULL;
  
  ClosestIntersection intersection = intersectClosest(O, D, t_min, t_max);
  closestSphere = intersection.sphere;
  closest_t = intersection.distance;


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
  // todo: clean up centerX ...
  N.x = P.x - closestSphere->centerX;
  N.y = P.y - closestSphere->centerY;
  N.z = P.z - closestSphere->centerZ;


  V3 local_color;
  local_color.x = closestSphere->color.x;
  local_color.y = closestSphere->color.y;
  local_color.z = closestSphere->color.z;

  V3 V;
  // todo: negate;
  // V from object to camera = -D from camera to object
  V.x = -D.x;
  V.y = -D.y;
  V.z = -D.z;
  float light = ComputeLighting(P,N,V,closestSphere->specular);

  local_color.x *= light;
  local_color.y *= light;
  local_color.z *= light;
  
  /* return local_color; */

  float reflective = closestSphere->reflective;
  if ( recursion_depth <= 0 || reflective <=0 ){
    return local_color;
  }

  // -D
  V3 R = ReflectRay(N,V);
  V3 reflected_color = traceRay(P, R, 0.001, INFINITY, recursion_depth-1);
  V3 result;
  result.x = local_color.x*(1-reflective) + reflected_color.x*reflective;
  result.y = local_color.y*(1-reflective) + reflected_color.y*reflective;
  result.z = local_color.z*(1-reflective) + reflected_color.z*reflective;
  
  return result;
}

void setPixelTexture(float x, float y, V3 color){
  int byteOffset = (x + WIDTH * y) * 3;
  // todo: better normalization
  buffer[byteOffset+0] = color.x > 255 ? 255 : color.x;
  buffer[byteOffset+1] = color.y > 255 ? 255 : color.y;
  buffer[byteOffset+2] = color.z > 255 ? 255 : color.z;  
}

void setPixelCanvas(float x, float y, V3 color){
  // in x - in Canvas space - w/2 to w/2
  // out x - texture coords  0 to w

  // in y - in Canvas space  [h/2 (top) , -h/2(bot)]
  // by - HEIGHT/2   [ 0 , -h]
  // and *-1 [0, h] according to texture space
  setPixelTexture( x+WIDTH/2, -(y-HEIGHT/2), color);
}

int main(int argc, char** argv){
  int widthPixel = WIDTH;
  int heightPixel = HEIGHT;

  // TOP RIGHT ++
  int bottomEdge = - heightPixel/2;
  int topEdge =  heightPixel/2;
  
  int leftEdge = - widthPixel/2;
  int righEdge = widthPixel/2;

  int index =0;
  V3 d, color;
  V3 o = {0};
  int recursion_depth = 3;
  for( int y = topEdge; y > bottomEdge; y--){
    index =0;
    for ( int x = leftEdge; x < righEdge; x++){

      /* V3 color = { index % 255, 0, 0 }; */
      /* setPixelCanvas(i,j, color); */
      d = canvasToViewport(x, y);
      color = traceRay( o, d, 1, INFINITY, recursion_depth);
      setPixelCanvas(x, y, color);

      index++;
    }
  }
  
  save(buffer, WIDTH, HEIGHT);
  
  return 0;
}

void save(unsigned char* buffer, int widthPixel, int heightPixel){
  FILE *fptr;
  fptr = fopen("out.ppm", "w");
  
  fprintf(fptr, "P3\n");
  fprintf(fptr, "%d %d\n", widthPixel, heightPixel);
  fprintf(fptr, "255\n");

  for( int j=0; j < heightPixel; j++){
    for ( int i=0; i< widthPixel; i++){
      int pixel = (i + WIDTH * j) *3;
      fprintf(fptr, "%d %d %d ", buffer[pixel +0], buffer[pixel+1], buffer[pixel+2]);
    }
  }
  
  fclose(fptr);
}
