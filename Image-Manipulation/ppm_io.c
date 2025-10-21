
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "ppm_io.h"
#include <math.h>


/* helper function for read_ppm, takes a filehandle
 * and reads a number, but detects and skips comment lines
 */
int read_num( FILE *fp ) {
  assert(fp);

  int ch;
  while((ch = fgetc(fp)) == '#') { // # marks a comment line
    while( ((ch = fgetc(fp)) != '\n') && ch != EOF ) {
      /* discard characters til end of line */
    }
  }
  ungetc(ch, fp); // put back the last thing we found

  int val;
  if (fscanf(fp, "%d", &val) == 1) { // try to get an int
    while(isspace(ch = fgetc(fp))) {
      // drop trailing whitespace
    }
    ungetc(ch, fp);
    return val; // we got a value, so return it
  } else {
    fprintf(stderr, "Error:ppm_io - failed to read number from file\n");
    return -1;
  }
}


Image read_ppm( FILE *fp ) {
  Image im = { NULL , 0 , 0 };
  
  /* confirm that we received a good file handle */
  if( !fp ){
    fprintf( stderr , "Error:ppm_io - bad file pointer\n" );
    return im;
  }

  int rows=-1 , cols=-1;

  /* read in tag; fail if not P6 */
  char tag[20];
  tag[19] = '\0';
  fscanf( fp , "%19s\n" , tag );
  if( strncmp( tag , "P6" , 20 ) ) {
    fprintf( stderr , "Error:ppm_io - not a PPM (bad tag)\n" );
    return im;
  }
  /* read image dimensions */

  //read in columns
  cols = read_num( fp ); // NOTE: cols, then rows (i.e. X size followed by Y size)
  //read in rows
  rows = read_num( fp );

  //read in colors; fail if not 255
  int colors = read_num( fp );
  if( colors!=255 ){
    fprintf( stderr , "Error:ppm_io - PPM file with colors different from 255\n" );
    return im;
  }

  //confirm that dimensions are positive
  if( cols<=0 || rows<=0 ){
    fprintf( stderr , "Error:ppm_io - PPM file with non-positive dimensions\n" );
    return im;
  }

  /* Allocate the new image */
  im = make_image( rows , cols );
  if( !im.data ){
    fprintf( stderr , "Error:ppm_io - Could not allocate new image\n" );
    return im;
  }
  /* finally, read in Pixels */

  /* read in the binary Pixel data */
  if( fread( im.data , sizeof(Pixel) , im.rows * im.cols , fp ) != (size_t)(im.rows * im.cols) ) {
    fprintf(stderr, "Error:ppm_io - failed to read data from file!\n");
    free_image( &im );
    return im;
  }

  //return the image struct pointer
  return im;
}




/* Write given image to disk as a PPM; assumes fp is not null */
int write_ppm(FILE *fp , const Image im ) {
  //write the tag  into the file as normal ie: P6 col x row and color
  fprintf(fp, "P6\n%d  %d\n255\n", im.cols, im.rows);
  fwrite(im.data, sizeof(Pixel), (im.cols) * (im.rows), fp);
  return im.rows * im.cols;

  
}


/* allocate a new image of the specified size;
 * doesn't initialize pixel values */


Image make_image( int rows , int cols ) {
  //allocate space for the image data
  Image im;
  im.rows = rows;
  im.cols = cols;
  im.data = malloc(sizeof(Pixel) * rows * cols);


  for (int i = 0; i < rows * cols; i++){
    im.data[i].r = 0;
    im.data[i].g = 0;
    im.data[i].b = 0;
  }
  
  return im;
}


/* output dimensions of the image to stdout */
void output_dims( const Image im ) {
  printf( "cols = %d, rows = %d" , im.cols , im.rows );
}

/* free_image
 * utility function to free inner and outer pointers, 
 * and set to null 
 */
void free_image( Image *im ) {
  //  if (im != NULL && im -> data != NULL){
  free(im -> data);
  // }
  im -> cols = 0;
  im -> rows = 0;

}



/* Helper function for blur in image_manip
 * reating the gaussian filter matrix
 */

double** createMatrix(int *n, double sigma) { 

  *n  = (int) (sigma * 10);//n should be big enough to hold at least 10 * sigma positions wide
  if (*n % 2 == 0) { //if size is even, make it odd
    (*n)++;
  }

  double** grid = (double**)malloc(sizeof(double) * (*n));//allocates the "rows of the allocated space
  for (int i = 0; i < *n; i++) {
    grid[i] = (double*)malloc(sizeof(double) * (*n));//for each row allocates a "column"
  }


  //beginning of image convolution
  int half = *n / 2;
  double count = 0.0;
  for (int dx = -half; dx <= half; ++dx) { // filter
    for (int dy = -half; dy <= half; ++dy) {

      double g = (1.0 / (2.0 * acos(-1) * (sigma * sigma)) * exp( -((dx * dx) + (dy * dy)) / (2 * (sigma * sigma))));
      grid[dx + half][dy + half] = g;
      count += g;
    }

  }
  
  return grid;

}

/* Helper function for blur in image_manip
 * applying blur to each pixel
 */
Pixel applyBlur(Image im, double** gaussian, int size, int dx, int dy) {
  double r = 0.0, g = 0.0, b = 0.0, total = 0.0;
  int half = size / 2;
  for (int i = -half; i <= half; i++) {
    for (int j = -half; j <= half; j++) {
      int x = dx + i, y = dy + j;
      if (x >= 0 && x < im.rows && y >= 0 && y < im.cols) {
	Pixel pixel = im.data[y + im.cols * x];
	double factor = gaussian[half + i][half + j];
	r += pixel.r * factor;
	g += pixel.g * factor;
	b += pixel.b * factor;
	total += factor;
      }
    }
  }
  Pixel resultPix = {(unsigned char)(r / total), (unsigned char)(g / total), (unsigned char)(b / total)};
    
  return resultPix;
}
