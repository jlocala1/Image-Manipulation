#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "image_manip.h"
#include "ppm_io.h"


////////////////////////////////////////
// Definitions of the functions       //
// declared in image_manip.h go here! //
////////////////////////////////////////
Image make_image(int rows, int cols);
double** createMatrix(int *n, double sigma);
Pixel applyBlur(Image im, double** gaussian, int size, int dx, int dy);


//______grayscale______                                                       
/* convert an image to grayscale (NOTE: pixels are still                      
 * RGB, but the three values will be equal)                                    
 */

Image grayscale( const Image in ) {

  unsigned char gray;
   
  //traverse through the space allocated for in which is laid out in a single line rather than a 2D array
  for(int i = 0; i < in.rows * in.cols; i++){
    //calculate the gray factor based on r b and g
    gray = (unsigned char)((0.3 * in.data[i].r) + (in.data[i].g * 0.59) + (in.data[i].b * 0.11));

    //gray each of the red blue and green values by makin1g it equal to teh gray factor
    in.data[i].r = gray;
    in.data[i].g = gray;
    in.data[i].b = gray;
  }
  return in;
}

/* _______alpha blend________                                                 
 * blend two images into one using the given alpha factor                      
 */
Image blend(const Image in1, const Image in2, double alpha) {
  
  int rowMAX = 0;
  int colMAX = 0;
  int rowMIN = 0;
  int colMIN = 0;
  
  // find maximum dimensions for larger image
  if (in1.cols > in2.cols) {
    colMAX = in1.cols;
  } 
  else {
    colMAX = in2.cols;
  }

  if (in1.rows > in2.rows) {
    rowMAX
 = in1.rows;
  } 
  else {
    rowMAX = in2.rows;
  }
  
  //find minimum dimensions of smaller image
  
  if (in1.cols < in2.cols) {
    colMIN = in1.cols;
  } 
  else {
    colMIN = in2.cols;
  }

  if (in1.rows < in2.rows) {
    rowMIN = in1.rows;
  } 
  else {
    rowMIN = in2.rows;
  }

  Image image = make_image(rowMAX, colMAX);
  
  // Initialize all pixels in new image to black
  for (int i = 0; i < rowMAX * colMAX; i++) {
    image.data[i].r = 0;
    image.data[i].g = 0;
    image.data[i].b = 0;
  }
  
  // blend area of smaller image
  for (int i = 0; i < rowMIN; i++) {
    for (int j = 0; j < colMIN; j++) {
      // change pixels 
      image.data[i * image.cols + j].r = (unsigned char)((in1.data[i * in1.cols + j].r * alpha) + (in2.data[i * in2.cols + j].r * (1 - alpha)));
      image.data[i * image.cols + j].g = (unsigned char)((in1.data[i * in1.cols + j].g * alpha) + (in2.data[i * in2.cols + j].g * (1 - alpha)));
      image.data[i * image.cols + j].b = (unsigned char)((in1.data[i * in1.cols + j].b * alpha) + (in2.data[i * in2.cols + j].b * (1 - alpha)));
    }
  }
  
  // assign pixels outside the blended area to their original image
  if (rowMAX > rowMIN) {
    for (int i = rowMIN; i < rowMAX; i++) { // area between smaller and larger image boundaries
      for (int j = 0; j < colMAX; j++) {
        if (i < in1.rows && j < in1.cols) {
          image.data[i * image.cols + j] = in1.data[i * in1.cols + j]; // result image at that spot is set to input image
        } 
        else if (i < in2.rows && j < in2.cols) {
          image.data[i * image.cols + j] = in2.data[i * in2.cols + j]; // result image at that spot is set to input image
        }
      }
    }
  }
  // do the same but with columns
  if (colMAX > colMIN) {
    for (int i = 0; i < rowMIN; i++) {
      for (int j = colMIN; j < colMAX; j++) {
        if (j < in1.cols) {
          image.data[i * image.cols + j] = in1.data[i * in1.cols + j];
        } 
        else if (j < in2.cols) {
          image.data[i * image.cols + j] = in2.data[i * in2.cols + j];
        }
      }
    }
  }
  
  return image;
}
  

/* _______rotate-ccw________                                                  
 * rotate the input image counter-clockwise                                    
 */
Image rotate_ccw(const Image in) {
  // Create a new image with the swapped dimensions                    
  Image rotated_image = make_image(in.cols, in.rows);

  // Transpose the image                                               
  for (int i = 0; i < in.rows; i++) {
    for (int j = 0; j < in.cols; j++) {
      rotated_image.data[j * rotated_image.cols + i].r = in.data[i * in.cols + j].r;
      rotated_image.data[j * rotated_image.cols + i].g = in.data[i * in.cols + j].g;
      rotated_image.data[j * rotated_image.cols + i].b = in.data[i * in.cols + j].b;
    }
  }

  // Flip the columns of the transposed image                             
  for (int i = 0; i < rotated_image.rows / 2; i++) {
    for (int j = 0; j < rotated_image.cols; j++) {
      // Swap the data in the rows                                 
      Pixel tempdata = rotated_image.data[i * rotated_image.cols + j];
      rotated_image.data[i * rotated_image.cols + j] = rotated_image.data[(rotated_image.rows - 1 - i) * rotated_image.cols + j];
      rotated_image.data[(rotated_image.rows - 1 - i) * rotated_image.cols + j] = tempdata;
    }
  }

  // Free the input image data 
  free(in.data);

  // Return the rotated image
  return rotated_image;
}

/* _______pointilism________                                                  
 * apply a painting like effect i.e. poitilism technique.                      
 */
Image pointilism(const Image in, unsigned int seed) {
  srand(seed); // create the standardized random

  int numPix = in.rows * in.cols; // total pixels because dynamic allocation uses one continuous array of memory
  int pointPix = (int)numPix * 0.03; // 3% of total grid to apply pointilism
  Image black_image = make_image(in.rows,in.cols);

  for (int i = 0; i < in.rows * in.cols; i++) { // initialize new image to black
    black_image.data[i].r = 0;
    black_image.data[i].g = 0;
    black_image.data[i].b = 0;
  }

  int radius;
  int randX;
  int randY;
  
  for (int i = 0; i < pointPix; i++) { // choose pixels to change using random seed
    radius = rand() % 5 + 1;
    randX = rand() % in.cols;
    randY = rand() % in.rows;

    // Get the color of the center pixel
    Pixel centerColor = in.data[randY * in.cols + randX];

    for (int j = -radius; j <= radius; j++) { // create a circle around each pixel to pointillate
      for (int k = -radius; k <= radius; k++) {
	if ((j * j) + (k * k) <= (radius * radius)) {
	  int resultY = randY + j;
	  int resultX = randX + k;
	  if (resultX >= 0 && resultY >= 0 && resultX < in.cols && resultY < in.rows) {
	    // Assign the adjusted color of the center pixel to the pixels within the radius
	    black_image.data[resultY * in.cols + resultX] =  centerColor;
	  }
	}
      }
    }


  }
  
  free(in.data); // free original image data
  return black_image; 
}


//______blur______                                                            
/* apply a blurring filter to the image                                       
 */
Image blur( const Image in , double sigma ) {
  
  if (sigma == 0) { //edge case
    return in;
  }
  int n;
  double **gaussian = createMatrix(&n, sigma); // generate gaussian matrix
  
  Image result = make_image(in.rows, in.cols); // create new image with same dimensions
    
  for (int x = 0; x < in.rows; x++) {
    for (int y = 0; y < in.cols; y++) {
      Pixel pix = applyBlur(in, gaussian, n, x, y); // iterate through pixels and apply blur
      result.data[x * result.cols + y] = pix; // Ensure result uses its own dimensions for indexing
    }
  }

  
  for (int i = 0; i < n; i++) { //free dynamically allocated matrix
    free(gaussian[i]);
  }
  free(gaussian);
  free(in.data); //free original image data

  return result;

}

//______saturate______                                                        
/* Saturate the image by scaling the deviation from gray                      
 */
Image saturate(const Image in, double scale) {
  for (int i = 0; i < in.rows * in.cols; i++) {
    // Compute the pixel's gray-scale value
    unsigned char gray = (unsigned char)(0.3 * in.data[i].r + 0.59 * in.data[i].g + 0.11 * in.data[i].b);

    //scaled difference
    double difference_red = ((double)((in.data[i].r) - gray) * scale) + gray;
    double difference_green = ((double)((in.data[i].g) - gray) * scale) + gray;
    double difference_blue = ((double)((in.data[i].b) - gray) * scale) + gray;

    //clamp the values
    if (difference_red <= 0){
      difference_red = 0;
    }
    
    if (difference_red >= 255){
      difference_red = 255;
    }
    if (difference_blue <= 0){
      difference_blue = 0;
    }
    if (difference_blue >= 255){
      difference_blue = 255;
    }
    if (difference_green <= 0){
      difference_green = 0;
    }
    if (difference_green >= 255){
      difference_green = 255;
    }
    
    //assigned the weigheted grayscale values to the data at i
    in.data[i].r = (unsigned char) ( difference_red);
    in.data[i].g =(unsigned char) (difference_green);
    in.data[i].b = (unsigned char) (difference_blue);

  }
  return in;
}
