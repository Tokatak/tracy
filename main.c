#include "tracer.h"
#include <stdio.h>
#include <windows.h> // PROFILE


int main(int argc, char **argv) {

  Sphere spheres[] = {
      // position
      {{0, -1, 3}, 1, {255, 0, 0}, 500, 0.2},
      {{2, 0, 4}, 1, {0, 0, 255}, 500, 0.3},
      {{-2, 0, 4}, 1, {0, 255, 0}, 10, 0.4},
      {{0, -5001, 0}, 5000, {255, 255, 0}, 1000, 0.5},
  };

  // todo: fix no ligth exception
  Light lights[] = {
    {LIGHT_AMBIENT, 0.2, {0, 0, 0}},
    {LIGHT_POINT, 0.6, {2, 1, 0}},
    {LIGHT_DIRECTIONAL, 0.2, {1, 4, 4}},
  };

  V3 viewportSize = {1.0, 1.0, 0.0};
  float projectionPlane = 1.0;

  Buffer buffer = {0};
  buffer.width = 640;
  buffer.height = 480;
  buffer.size = buffer.width * buffer.height * 3;

  unsigned char *b = (unsigned char *)malloc((size_t)buffer.size);
  if (b == NULL) {
    return -1;
  }
  buffer.start = b;

  // TOP +
  // RIGHt +
  int topEdge = buffer.height / 2;
  int bottomEdge = -buffer.height / 2;

  int leftEdge = -buffer.width / 2;
  int righEdge = buffer.width / 2;
  Region region;
  region.top = topEdge;
  region.bot = bottomEdge;
  region.left = leftEdge;
  region.right = righEdge;

  V3 direction = {0}, color = {0}, origin = {0};
  int recursion_depth = 3;

  // profile
  LARGE_INTEGER frequency, start, end;
  double elapsed;
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&start);
  // profile

  fillRegion( origin, region, viewportSize, projectionPlane, buffer,
	      1, INFINITY, recursion_depth,
	      spheres, ARRAY_SIZE(spheres), lights, ARRAY_SIZE(lights));
	        
  // profile
  QueryPerformanceCounter(&end);
  elapsed = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
  printf("Frame time: %.6f seconds\n", elapsed);
  printf("FPS: %.2f\n", 1.0 / elapsed);
  // profile

  save(buffer.start, buffer.width, buffer.height, "out.ppm");

  free(buffer.start);
  return 0;
}
