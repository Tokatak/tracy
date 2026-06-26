#include <stdio.h>
#include <math.h>

#define ARRAY_SIZE(arr)(sizeof(arr)/sizeof((arr)[0]))

// 640 480
#define WIDTH 640
#define HEIGHT 480
#define INFINITY 999999

#define DEFAULT_COLOR (V3){255,255,255}
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

V2 viewportSize = {1.0,1.0};
float projectionPlane = 1.0;

Sphere spheres[] = {
  { 0, -1, 3,  1,  {255,   0,  0}},
  { 2,  0, 4,  1,  {  0,   0,255}},
  {-2,  0, 4,  1,  {  0, 255,  0}}
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

V3 traceRay( V3 O, V3 D, float t_min, float t_max ){
  V3 color = {0};
  float t_closest = INFINITY;
  Sphere *closestSphere = NULL;
  
  RaySphereIntersection hit;

  for (int i = 0; i < ARRAY_SIZE(spheres); i++){    
    hit = intersectRaySphere(O, D, spheres[i]);

    if( hit.count == 0 ) {
      continue;
    }

    if( hit.count == 1 ){
      if ( hit.t1 > t_min && hit.t1 < t_max && hit.t1< t_closest){
	t_closest = hit.t1;
	closestSphere = &spheres[i];
      }
    }

    if( hit.count == 2 ){
      if ( hit.t2 > t_min && hit.t2 < t_max && hit.t2< t_closest){
	t_closest = hit.t2;
	closestSphere = &spheres[i];
      }
    }

  }

  if(closestSphere!=NULL){
    return closestSphere->color;
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
