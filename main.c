#include <stdio.h>

#define ARRAY_SIZE(arr)(sizeof(arr)/sizeof((arr)[0]))

// 640 480
#define WIDTH 640
#define HEIGHT 480
unsigned char buffer[WIDTH*HEIGHT*3] = { 0 };

void save(unsigned char* buffer, int widthPixel, int heightPixel);

typedef struct{
  float centerX;
  float centerY;
  float radius;
  char r;
  char g;
  char b;
} circle;

circle circles[] = {
  {200, 200, 200,   0, 255,0},
  {400, 200, 100 , 255, 0  ,0},
};

int main(int argc, char** argv){
  int widthPixel = WIDTH;

  int heightPixel = HEIGHT;
  for( int j=0; j < heightPixel; j++){
    for ( int i=0; i < widthPixel; i++){
      int byteOffset = (i + WIDTH * j) * 3;
      
      buffer[byteOffset+0] = 0;
      buffer[byteOffset+1] = 0;
      buffer[byteOffset+2] = 0;
      
    }
  }

  int centerX=0, centerY=0, radius=0;
  int circleX = 0, circleY =0;
  char r=0,g=0,b=0;
  for( int i =0; i < ARRAY_SIZE(circles); i++){
    circle c = circles[i];
    centerX = c.centerX;
    centerY = c.centerY;
    radius = c.radius;
    r = c.r;
    g = c.g;
    b = c.b;

    for ( int x = centerX - radius; x< centerX + radius; x++){
      for ( int y = centerY - radius; y < centerY+radius; y++){

	circleX = x - centerX;
	circleY = y - centerY;
	if( x > 0 & x < WIDTH & y> 0 & y< HEIGHT & circleX*circleX+circleY*circleY < radius*radius){
	  int byteOffset = (x + WIDTH * y) * 3;      
	  buffer[byteOffset+0] = r;
	  buffer[byteOffset+1] = g;
	  buffer[byteOffset+2] = b;

	}
      }
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
