#include <stdio.h>

// 640 480
#define WIDTH 640
#define HEIGHT 480
unsigned char buffer[WIDTH*HEIGHT*3]={};

void save(unsigned char* buffer, int widthPixel, int heightPixel);

int main(int argc, char** argv){
  int widthPixel = WIDTH;

  int heightPixel = HEIGHT;
  for( int j=0; j < heightPixel; j++){
    for ( int i=0; i < widthPixel; i++){
      
      int byteOffset = (i + WIDTH * j) * 3;
      
      buffer[byteOffset+0] = 120;
      buffer[byteOffset+1] = 120;
      buffer[byteOffset+2] = 120;
      
      if( i == j ){
	  buffer[byteOffset+0] = 255;
	  buffer[byteOffset+1] = 0;
	  buffer[byteOffset+2] = 0;
	  continue;
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
