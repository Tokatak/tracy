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
} Sphere;

typedef struct{
  int count;
  float t1;
  float t2;
} RaySphereIntersection;

typedef enum {AMBIENT, POINT, DIRECTIONAL} LIGHT_TYPE;

typedef struct{
  LIGHT_TYPE type;
  float intensity;
  V3 position;
} Light;


V2 viewportSize = {1.0,1.0};
float projectionPlane = 1.0;

Sphere spheres[] = {
  { 0, -1, 3,  1,  {255,   0,  0}},
  { 2,  0, 4,  1,  {  0,   0,255}},
  {-2,  0, 4,  1,  {  0, 255,  0}},
  {0,-5001,0,5000, {255, 255,  0}},  
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

float ComputeLighting(V3 P, V3 N){
  float intensity = 0.0;
  V3 L;

  for( int i =0; i< ARRAY_SIZE(lights); i++){
    Light l = lights[i];
    
    if (l.type == AMBIENT){
      intensity += l.intensity;
      continue;
    }

    if ( l.type == POINT ){
      // todo: v3.sub
      L.x = l.position.x - P.x;
      L.y = l.position.y - P.y;
      L.z = l.position.z - P.z;
    } else { // DIRECTIONAL
      // todo: v3.set
      L.x = l.position.x;
      L.y = l.position.y;
      L.z = l.position.z;
    }

    float nDotl = dot( N, L);
    if ( nDotl > 0 ){
      intensity += l.intensity * nDotl / (len(N) * len(L)) ;
    }    
  }

  return intensity;
}


V3 traceRay( V3 O, V3 D, float t_min, float t_max ){
  V3 color = {0};
  float closest_t = INFINITY;
  Sphere *closestSphere = NULL;
  
  RaySphereIntersection hit;

  for (int i = 0; i < ARRAY_SIZE(spheres); i++){    
    hit = intersectRaySphere(O, D, spheres[i]);

    if( hit.count == 0 ) {
      continue;
    }

    if( hit.count == 1 ){
      if ( hit.t1 > t_min && hit.t1 < t_max && hit.t1< closest_t){
	closest_t = hit.t1;
	closestSphere = &spheres[i];
      }
    }

    if( hit.count == 2 ){
      if ( hit.t2 > t_min && hit.t2 < t_max && hit.t2< closest_t){
	closest_t = hit.t2;
	closestSphere = &spheres[i];
      }
    }

  }

  // no light 
  /* if(closestSphere!=NULL){ */
  /*   return closestSphere->color; */
  /* } */

  // Lit
  if( closestSphere!=NULL){
    V3 P;
    P.x = O.x + closest_t * D.x;
    P.y = O.y + closest_t * D.y;
    P.z = O.z + closest_t * D.z;

    V3 N;
    // todo: clean up centerX ...
    N.x = P.x - closestSphere->centerX;
    N.y = P.y - closestSphere->centerY;
    N.z = P.z - closestSphere->centerZ;


    V3 outColor;
    outColor.x = closestSphere->color.x;
    outColor.y = closestSphere->color.y;
    outColor.z = closestSphere->color.z;

    float light = ComputeLighting(P,N);

    outColor.x *= light;
    outColor.y *= light;
    outColor.z *= light;
  
    return outColor;
  }

  return DEFAULT_COLOR;
}

void setPixelTexture(float x, float y, V3 color){
  int byteOffset = (x + WIDTH * y) * 3;
  buffer[byteOffset+0] = color.x;
  buffer[byteOffset+1] = color.y;
  buffer[byteOffset+2] = color.z;  
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
  for( int y = topEdge; y > bottomEdge; y--){
    index =0;
    for ( int x = leftEdge; x < righEdge; x++){

      /* V3 color = { index % 255, 0, 0 }; */
      /* setPixelCanvas(i,j, color); */
      d = canvasToViewport(x, y);
      color = traceRay( o, d, 1, INFINITY);
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
