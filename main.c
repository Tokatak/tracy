#include <stdio.h>
#include <windows.h> // PROFILE
#include "include/utility.h"
#include "include/v3.h"
#include "include/ray.h"
#include "include/tracer.h"
#include "include/ppm.h"


V3 canvasToViewport(float x, float y, float width, float height, V3 viewportSize, float projectionPlane){
  V3 result = {0};
  result.x = x * viewportSize.x / width;
  result.y = y * viewportSize.y / height;
  result.z = projectionPlane;
  
  return result;;
}

void setPixelTexture(float x, float y, V3 color, Buffer* buffer){
  int byteOffset = (x + buffer->width * y) * 3;
  // potential branchless operation
  // clamp [0, 255]
  buffer->start[byteOffset+0] = (unsigned char)fmaxf( 0.0, fminf( color.x, 255.0));
  buffer->start[byteOffset+1] = (unsigned char)fmaxf( 0.0, fminf( color.y, 255.0));
  buffer->start[byteOffset+2] = (unsigned char)fmaxf( 0.0, fminf( color.z, 255.0));
}

void setPixelCanvas(float x, float y, V3 color, Buffer* buffer){
  // in x - in Canvas space - w/2 to w/2
  // out x - texture coords  0 to w

  // in y - in Canvas space  [h/2 (top) , -h/2(bot)]
  // by - HEIGHT/2   [ 0 , -h]
  // and *-1 [0, h] according to texture space
  setPixelTexture( x+(buffer->width)/2, -(y-(buffer->height)/2), color, buffer);
}


int main(int argc, char** argv){

  Sphere spheres[] = {
    //position
    { {0,   -1, 3},    1, {255,   0,  0}, 500, 0.2},
    { {2,    0, 4},    1, {  0,   0,255}, 500, 0.3},
    { {-2,   0, 4},    1, {  0, 255,  0}, 10 , 0.4},
    { {0,-5001, 0}, 5000, {255, 255,  0},1000, 0.5},  
  };

  Light lights[]= {
    {LIGHT_AMBIENT,     0.2, {0,0,0} },
    {LIGHT_POINT,       0.6, {2, 1, 0}},
    {LIGHT_DIRECTIONAL, 0.2, {1,4,4}},
  };

  V3 viewportSize = {1.0,1.0,0.0};
  float projectionPlane = 1.0;


  Buffer buffer = {0};
  buffer.width = 640;
  buffer.height = 480;
  buffer.size = buffer.width * buffer.height*3;

  unsigned char* b = (unsigned char*)malloc((size_t)buffer.size);
  if (b == NULL) {
    return -1;
  }
  buffer.start = b;

  // TOP +
  // RIGHt +
  int topEdge =  buffer.height/2;
  int bottomEdge = - buffer.height/2;

  int leftEdge = - buffer.width/2;
  int righEdge = buffer.width/2;

  V3 direction = {0}, color = {0}, origin = {0};
  int recursion_depth = 3;

  // profile
  LARGE_INTEGER frequency, start, end;
  double elapsed;
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&start);
  // profile
    
  for( int y = topEdge; y > bottomEdge; y--){
    for ( int x = leftEdge; x < righEdge; x++){
      direction = canvasToViewport(x, y, buffer.width, buffer.height, viewportSize, projectionPlane);
      color = traceRay( origin, direction, 1, INFINITY, recursion_depth, spheres, ARRAY_SIZE(spheres),
			lights, ARRAY_SIZE(lights));
      setPixelCanvas(x, y, color, &buffer);
    }
  }
  
  // profile
  QueryPerformanceCounter(&end);
  elapsed = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
  printf("Frame time: %.6f seconds\n", elapsed);
  printf("FPS: %.2f\n", 1.0 / elapsed);
  // profile
  
  save(buffer.start, buffer.width, buffer.height,"out.ppm");

  free(buffer.start);
  return 0;
}

