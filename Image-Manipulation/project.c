//project.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ppm_io.h"
#include "image_manip.h"
#include <ctype.h>

// Return (exit) codes
#define RC_SUCCESS            0
#define RC_MISSING_FILENAME   1
#define RC_OPEN_FAILED        2
#define RC_INVALID_PPM        3
#define RC_INVALID_OPERATION  4
#define RC_INVALID_OP_ARGS    5
#define RC_OP_ARGS_RANGE_ERR  6
#define RC_WRITE_FAILED       7
#define RC_UNSPECIFIED_ERR    8


void print_usage();

int main (int argc, char* argv[]) {
  
  //if the command line doesnt have at least one arguments (the file name) it should return RC_MISSING_FILE  
  if (argc < 2) {
    fprintf(stderr, "ERROR: the filename was not provided\n"); 
    return RC_MISSING_FILENAME;
  }
  //if there aren't at least 3 arguments then an operation is not specified                                                                                                         
  if (argc < 3) {
    fprintf(stderr, "Did not specify an operation\n");
    return RC_OPEN_FAILED;
  }
  
  if (argc < 4 ) {
    fprintf(stderr,"No operation given\n");
    return RC_INVALID_OPERATION;
  }
  if (strstr(argv[1], ".ppm") == NULL || strstr(argv[2], ".ppm") == NULL) {
    fprintf(stderr, "Input file(s) do not contain '.ppm'\n");
    return RC_WRITE_FAILED;
  }
  //consider blend edge case (this requires 2 files to be present initially)
  char *operation = argv[3];
  if((operation[0] == 'b')){                                                                                                                                                        
    if((operation[2] == 'e')){
      if (argc < 4){
	fprintf(stderr,"missing a second file name for blend\n");
	return RC_MISSING_FILENAME;
      }
    }
  }
  
  //open a file for reaidng binary based on the command line
  FILE *fp = fopen(argv[1], "rb+");
  if ( fp == NULL){
    fprintf(stderr, "invalid file entered\n");
    return RC_OPEN_FAILED;
  }
  Image change_image;
  Image input_image = read_ppm(fp);
  change_image = make_image(input_image.rows, input_image.cols);

  //load teh original r,g,b channels from iput into chnage_image;
  for(int i = 0; i  < input_image.rows * input_image.cols; i++){
    change_image.data[i].r = input_image.data[i].r;
    change_image.data[i].g = input_image.data[i].g;
    change_image.data[i].b = input_image.data[i].b;
  }
  
  free_image(&input_image);
  fclose(fp);
  
  if(change_image.data == NULL){//if the file entered into the command line doesn't have data return ERROR
    free_image(&change_image);
    fprintf(stderr, "the file you have inputed contains incorrect image data");
    return RC_INVALID_PPM;
  }

  //chain of command to figure out what to do in  each operation case

  //applying grayscale
  if(strcmp(operation, "grayscale") == 0){
    if(change_image.data == NULL){
      fprintf(stderr, "You have not provided a proper ppm_file to be opened");
      free_image(&change_image);
      return RC_INVALID_PPM;
    }
    if (argc != 4) {
      free_image(&change_image);
      fprintf(stderr, "Invalid number of arguments\n");
      return RC_INVALID_OP_ARGS;
    }
      
    else {
      change_image = grayscale(change_image);
      FILE *fp1 = fopen(argv[2], "wb");
      if(fp1 == NULL){
	fprintf(stderr, "write_ppm failed.\n");
	free_image(&change_image);
	//fclose(fp1);
	return RC_WRITE_FAILED;
      }
      if (strstr(argv[2], ".ppm") == NULL) {
	fprintf(stderr, "Output file does not contain '.ppm' extension");
	free_image(&change_image);
	fclose(fp1);
	return RC_WRITE_FAILED;
      }
	  
      write_ppm(fp1, change_image);
      fclose(fp1);
    }	
    free_image(&change_image);
    return RC_SUCCESS;
      
  }
  
  if(strcmp(operation, "blend") == 0){
    FILE *fp1 = fopen(argv[1], "rb+");
    FILE *fp2 = fopen(argv[2], "rb+");

    if (fp1 == NULL && fp2 == NULL) {
      fprintf(stderr, "Invalid file entered\n");
      //fclose(fp2);                                                                                                                                                               
      //fclose(fp2);                                                                                                                                                               
      free_image(&change_image);
      return RC_OPEN_FAILED;
    }
    else if (fp1 != NULL && fp2 == NULL ){
      fclose(fp1);
      free_image(&change_image);
      fprintf(stderr, "Invalid file entered\n");
      return RC_OPEN_FAILED;

    }
    else if (fp2 == NULL && fp1 == NULL){
      fclose(fp2);
      free_image(&change_image);
      fprintf(stderr, "Invalid file entered\n");
      return RC_OPEN_FAILED;
    }

    if (argc != 6) {
      fclose(fp1);
      fclose(fp2);
      free_image(&change_image);
      fprintf(stderr, "Invalid number of arguments\n");
      return RC_INVALID_OP_ARGS;
    }
    float alpha  = atof(argv[5]);
    if(alpha == 0 && strcmp(argv[5], "0") != 0 && strcmp(argv[5], "0.0") != 0){//consider edge case in which value is 0
      fprintf(stderr, "invalid argument type\n"); // command line argument expects a number, reads something else
      fclose(fp1);
      fclose(fp2);
      free_image(&change_image);
      return RC_OP_ARGS_RANGE_ERR;
    } 
        
    Image in1 = read_ppm(fp1);
    Image in2 = read_ppm(fp2);
    alpha = atof(argv[5]);
	
    free_image(&change_image);
    change_image = blend(in1, in2 , alpha);
    fclose(fp1);
    fclose(fp2);
    free_image(&in1);
    free_image(&in2);	

    if(change_image.rows == 0 && change_image.cols == 0){
      fprintf(stderr, "You have not provided a proper ppm_file to be opened");
      free_image(&change_image);
      return RC_WRITE_FAILED;
    }
	
    else {
      FILE *fp1 = fopen(argv[4], "wb");
      if(fp1 == NULL){
	fprintf(stderr, "write_ppm failed.\n");
	fclose(fp1);
	return RC_WRITE_FAILED;
      }
      if (strstr(argv[4], ".ppm") == NULL) {
	fprintf(stderr, "Output file does not contain '.ppm' extension");
	fclose(fp1);
	return RC_WRITE_FAILED;
      }
      write_ppm(fp1, change_image);	  
      fclose(fp1);
      free_image(&change_image);
      return RC_SUCCESS;
    }	
  }
    
  //apply the blur function
   
  else if(strcmp(operation, "blur") == 0){
    if (argc != 5) {
      free_image(&change_image);
      fprintf(stderr, "Invalid number of arguments\n");
      return RC_INVALID_OP_ARGS;
    }
    float sigma  = atof(argv[4]);
    if(sigma == 0 && strcmp(argv[4], "0") != 0 && strcmp(argv[4], "0.0") != 0){
      fprintf(stderr, "invalid argument type\n"); // command line argument expects a number, reads something else
      free_image(&change_image);
      return RC_OP_ARGS_RANGE_ERR;
    }
      
    change_image =  blur(change_image, atof(argv[4]));
      

    if(change_image.data == NULL){
      fprintf(stderr, "You have not provided a proper ppm_file to be opened");
      free_image(&change_image);
      return RC_WRITE_FAILED;
    }
	
    else {
      FILE *fp1 = fopen(argv[2], "wb");
      if(fp1 == NULL){
	fprintf(stderr, "write_ppm failed.\n");
	fclose(fp1);
	return RC_WRITE_FAILED;
      }
      if (strstr(argv[2], ".ppm" ) == NULL) {
	fprintf(stderr, "Output file does not contain '.ppm' extension");
	fclose(fp1); // check for valgrind
	return RC_WRITE_FAILED;
      }
	    
      write_ppm(fp1, change_image);
      fclose(fp1);
      free_image(&change_image);
      return RC_SUCCESS;
    }
  }
      
  //applying the pointilism function
  
  if(strcmp(operation, "pointilism") == 0){
    if (argc != 4) {
      free_image(&change_image);
      fprintf(stderr, "Invalid number of arguments\n");
      return RC_INVALID_OP_ARGS;
    }
    change_image = pointilism(change_image, 1);//the seed value is supposed to be 1

    if(change_image.data == NULL){
      fprintf(stderr, "You have not provided a proper ppm_file to be opened");
      free_image(&change_image);
      return RC_WRITE_FAILED;
    }
	
    else {
      FILE *fp1 = fopen(argv[2], "wb");
      if(fp1 == NULL){
	fprintf(stderr, "write_ppm failed.\n");
	fclose(fp1);
	return RC_WRITE_FAILED;
      }
      if (strstr(argv[2], ".ppm") == NULL) {
	fprintf(stderr, "Output file does not contain '.ppm' extension");
	fclose(fp1);
	return RC_WRITE_FAILED;
      }
      write_ppm(fp1, change_image);
      fclose(fp1);
      free_image(&change_image);
      return RC_SUCCESS;
    }
  }
  

  //applying the saturate function
  if(strcmp(operation, "saturate") == 0){
    if (argc != 5) {
      free_image(&change_image);
      fprintf(stderr, "Invalid number of arguments\n");
      return RC_INVALID_OP_ARGS;
    }
    double scale = atof(argv[4]);

    if(scale == 0 && strcmp(argv[4], "0") != 0 && strcmp(argv[4], "0.0") != 0){
      fprintf(stderr, "invalid argument type\n"); // command line argument expects a number, reads something else
      free_image(&change_image);
      return RC_OP_ARGS_RANGE_ERR;
    }
    change_image = saturate(change_image, atof(argv[4]));

    if(change_image.data == NULL){
      fprintf(stderr, "You have not provided a proper ppm_file to be opened");
      free_image(&change_image);
      return RC_WRITE_FAILED;
    }

    else {
      FILE *fp1 = fopen(argv[2], "wb");
      if(fp1 == NULL){
	fprintf(stderr, "write_ppm failed.\n");
	fclose(fp1);
	free_image(&change_image);
	return RC_WRITE_FAILED;
      }
      if (strstr(argv[2], ".ppm") == NULL) {
	fprintf(stderr, "Output file does not contain '.ppm' extension");
	free_image(&change_image);
	fclose(fp1);
	return RC_WRITE_FAILED;
      }
      write_ppm(fp1, change_image);
      fclose(fp1);
      free_image(&change_image);
      return RC_SUCCESS;
    }

  }

  //applying the rotate function
  if (strcmp(operation, "rotate-ccw") == 0) {
    if (argc != 4) {
      free_image(&change_image);
      fprintf(stderr, "Invalid number of arguments\n");
      return RC_INVALID_OP_ARGS;
    }
    Image rotated_image = rotate_ccw(change_image);
      
    if (rotated_image.data == NULL) {
      fprintf(stderr, "You have not provided a proper ppm_file to be opened");
      free_image(&rotated_image);
      return RC_WRITE_FAILED;
    }
      
    else {
      FILE *fp1 = fopen(argv[2], "wb");

      if(fp1 == NULL){
	fprintf(stderr, "write_ppm failed.\n");
	free_image(&rotated_image);
	fprintf(stderr, "failed to write ppm file");
	return RC_WRITE_FAILED;
      }

      else if (strstr(argv[2], ".ppm") == NULL) {
	fprintf(stderr, "Output file does not contain '.ppm' extension");
	fclose(fp1);
	free_image(&rotated_image);
	fprintf(stderr, "not a valid ppm file name\n");
	return RC_WRITE_FAILED;
      }
		
      else {
	write_ppm(fp1, rotated_image);
	fclose(fp1);
	free_image(&rotated_image);
	return RC_SUCCESS;
      }
    }
  }

    
  else {
    fprintf(stderr, "Invalid operation\n");
    free_image(&change_image);
    return RC_INVALID_OPERATION;
  }

}
void print_usage() {
  printf("USAGE: ./project <input-image> <output-image> <command-name> <command-args>\n");
  printf("SUPPORTED COMMANDS:\n");
  printf("   grayscale\n" );
  printf("   blend <target image> <alpha value>\n" );
  printf("   rotate-ccw\n" );
  printf("   pointilism\n" );
  printf("   blur <sigma>\n" );
  printf("   saturate <scale>\n" );
}
