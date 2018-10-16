
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pmmintrin.h>


// Define output file name
#define OUTPUT_FILE "stencil.pgm"

#define type float
#define ALIGN 32

// Define constant expressions
#define NUM_3_DIV_5 0.6
#define NUM_05_DIV_5 0.1

void stencil(const int nx, const int ny, type * restrict image, type * restrict tmp_image);
void init_image(const int nx, const int ny, type *  image, type *  tmp_image);
void output_image(const char * file_name, const int nx, const int ny, type *image);
double wtime(void);

int main(int argc, char *argv[]) {

  // Check usage
  if (argc != 4) {
    fprintf(stderr, "Usage: %s nx ny niters\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // Initiliase problem dimensions from command line arguments
  int nx = atoi(argv[1]);
  int ny = atoi(argv[2]);
  int niters = atoi(argv[3]);

  // Allocate the image
  type *image = _mm_malloc(sizeof(type)*nx*ny, ALIGN);
  type *tmp_image = _mm_malloc(sizeof(type)*nx*ny, ALIGN);

  // Set the input image
  init_image(nx, ny, image, tmp_image);

  // Call the stencil kernel
  double tic = wtime();
  for (int t = 0; t < niters; ++t) {
    stencil(nx, ny, image, tmp_image);
    stencil(nx, ny, tmp_image, image);
  }
  double toc = wtime();


  // Output
  printf("------------------------------------\n");
  printf(" runtime: %lf s\n", toc-tic);
  printf("------------------------------------\n");

  output_image(OUTPUT_FILE, nx, ny, image);
  _mm_free(image);
  _mm_free(tmp_image);
}

void stencil(const int nx, const int ny, type * restrict  image, type * restrict  tmp_image) {
  int size = nx*ny;
  type *ival = image;
  type *tmp_ival = tmp_image;
  int ny_min_1 = ny-1;
  int size_min_ny = size-ny;

  int i=0;
  for(;i<=ny;++i){
    (*tmp_ival) = (*ival) * NUM_3_DIV_5 + (*(ival+ny)) * NUM_05_DIV_5;
    if(i%ny) (*tmp_ival) += (*(ival-1)) * NUM_05_DIV_5;
    if(i%ny!=ny_min_1) (*tmp_ival) += (*(ival+1)) * NUM_05_DIV_5;
    ++ival;
    ++tmp_ival;
  }

  for(;i<size_min_ny;++i){
     (*tmp_ival) = (*ival) * NUM_3_DIV_5 + (*(ival-ny)) * NUM_05_DIV_5 + (*(ival+ny)) * NUM_05_DIV_5;
     if(i%ny) (*tmp_ival) += (*(ival-1)) * NUM_05_DIV_5;
     if(i%ny!=ny_min_1) (*tmp_ival) += (*(ival+1)) * NUM_05_DIV_5;
     //if(i>ny) (*tmp_ival) += (*(ival-ny)) * NUM_05_DIV_5;
//     if(i<size_min_ny) (*tmp_ival) += (*(ival+ny)) * NUM_05_DIV_5;
     ++ival;
     ++tmp_ival;
  }

  for(;i<size;++i){
    (*tmp_ival) = (*ival) * NUM_3_DIV_5 + (*(ival-ny)) * NUM_05_DIV_5;
    if(i%ny) (*tmp_ival) += (*(ival-1)) * NUM_05_DIV_5;
    if(i%ny!=ny_min_1) (*tmp_ival) += (*(ival+1)) * NUM_05_DIV_5;
    ++ival;
    ++tmp_ival;
  }


/*  for (int j = 0; j < ny; ++j) {
    for (int i = 0; i < nx; ++i) {
//      tmp_image[j+i*ny] = image[j+i*ny] * NUM_3_DIV_5;
      //if (i > 0)    tmp_image[j+i*ny] += image[j  +(i-1)*ny] * NUM_05_DIV_5;
      // if (i < nx-1) tmp_image[j+i*ny] += image[j  +(i+1)*ny] * NUM_05_DIV_5;
      //if (j > 0)    tmp_image[j+i*ny] += image[j-1+i*ny] * NUM_05_DIV_5;
//      if (j < ny-1) tmp_image[j+i*ny] += image[j+1+i*ny] * NUM_05_DIV_5;
    }
  } */
}

// Create the input image
void init_image(const int nx, const int ny, type *  image, type *  tmp_image) {
  // Zero everything
  for (int j = 0; j < ny; ++j) {
    for (int i = 0; i < nx; ++i) {
      image[j+i*ny] = 0.0;
      tmp_image[j+i*ny] = 0.0;
    }
  }

  // Checkerboard
  for (int j = 0; j < 8; ++j) {
    for (int i = 0; i < 8; ++i) {
      for (int jj = j*ny/8; jj < (j+1)*ny/8; ++jj) {
        for (int ii = i*nx/8; ii < (i+1)*nx/8; ++ii) {
          if ((i+j)%2)
          image[jj+ii*ny] = 100.0;
        }
      }
    }
  }
}

// Routine to output the image in Netpbm grayscale binary image format
void output_image(const char * file_name, const int nx, const int ny, type *image) {

  // Open output file
  FILE *fp = fopen(file_name, "w");
  if (!fp) {
    fprintf(stderr, "Error: Could not open %s\n", OUTPUT_FILE);
    exit(EXIT_FAILURE);
  }

  // Ouptut image header
  fprintf(fp, "P5 %d %d 255\n", nx, ny);

  // Calculate maximum value of image
  // This is used to rescale the values
  // to a range of 0-255 for output
  type maximum = 0.0;
  for (int j = 0; j < ny; ++j) {
    for (int i = 0; i < nx; ++i) {
      if (image[j+i*ny] > maximum)
        maximum = image[j+i*ny];
    }
  }

  // Output image, converting to numbers 0-255
  for (int j = 0; j < ny; ++j) {
    for (int i = 0; i < nx; ++i) {
      fputc((char)(255.0*image[j+i*ny]/maximum), fp);
    }
  }

  // Close the file
  fclose(fp);

}

// Get the current time in seconds since the Epoch
double wtime(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec*1e-6;
}
