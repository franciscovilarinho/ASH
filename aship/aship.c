/*
 ASH 2024-2025
 Final project
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Image size: use this size for IP implementation
// Adjust to the real image size for profiling on the PC
#define NROWS  512
#define NCOLS  512

// Use this for image FEUPin.png
//#define NROWS  1844
//#define NCOLS  4000

// Maximum size of the convolution kernel
#define MAXK   7


// Comment this line to use the input data from the include file datain.h
// Uncomment to load data from file and write data to file
#define LOAD_FROM_FILE


// Input image:
// load from local file or use the constant data read from the include file datain.h
#ifndef LOAD_FROM_FILE
#include "datain.h"
#endif

#ifdef LOAD_FROM_FILE
int Min[NROWS][NCOLS];
#endif

// The output image
volatile int Mout[NROWS][NCOLS];

// Temporary matrices:
int Ma[NROWS][NCOLS];
int Mb[NROWS][NCOLS];

// Convolution kernel and kernel size:
int kernel[MAXK][MAXK];


//-------------------------------------------------------------------
#ifdef LOAD_FROM_FILE
void
LoadData( char *filename, int Min[NROWS][NCOLS] )
{
	FILE *fpin;
	int x;
	int i, j;
	
	if( ( fpin = fopen( filename, "r" ) ) == NULL )
	{
		printf("Input file %s not found. Exiting...\n", filename );
		exit( -1 );
	}

    // Init	matrix:
	for(i=0; i<NROWS; i++)
		for(j=0; j<NCOLS; j++)
			Min[i][j] = 0;
		
	// Load matrix from text file:
	for(i=0; i<NROWS; i++)
		for(j=0; j<NCOLS; j++)
		{
			if ( fscanf(fpin, "%d", &x ) != 1 )
				return;
			Min[i][j] = x;		
		}	
	fclose( fpin );
}
#endif


//-------------------------------------------------------------------
int
SaturateToUint8( int Min[NROWS][NCOLS],  // Input image
                 int Mout[NROWS][NCOLS]  // Output image
			   )
{  
  int i, j;
  
  for(i=0; i < NROWS; i++ )
    for(j=0; j < NCOLS; j++ )
      Mout[i][j] = Min[i][j] > 255 ? 255 : ( Min[i][j] < 0 ? 0 : Min[i][j] );
}


//-------------------------------------------------------------------
int
ScaleImageAndSaturateToUint8( 
            int Min[NROWS][NCOLS],  // Input image
            int Mout[NROWS][NCOLS], // Output image
			int scale               // Scale factor multiplied by 4096 (12 factional bits, signed)
	      )
{  
  int i, j;
  
  for(i=0; i < NROWS; i++ )
    for(j=0; j < NCOLS; j++ )
      Mout[i][j] = ( Min[i][j] * scale ) >> 12;
  
  SaturateToUint8( Mout, Mout );
}



//-------------------------------------------------------------------
void
ConvolveImage( int Min[NROWS][NCOLS],		// Input image
               int Mout[NROWS][NCOLS],      // Output image
			   int kernel[MAXK][MAXK],      // kernel to convolve with, up to MAXK x MAXK
               int KernelSize,              // Number of rows and cols of kernel (must be odd)
			   int scalef                   // scale factor, integer, factor * 4096
			 )
{
	int i, j, k, l; // loop indexes
    int pixout;     // Pixel output

	for(i=0+KernelSize/2; i < NROWS - KernelSize/2; i++ )
	   for(j=0+KernelSize/2; j < NCOLS - KernelSize/2; j++ )
	   {
		   pixout = 0;
		   for( k=-KernelSize/2; k<=KernelSize/2; k++ )
			   for( l=-KernelSize/2; l<=KernelSize/2; l++ )
			   {
				   pixout = pixout + kernel[k+KernelSize/2][l+KernelSize/2] * Min[i+k][j+l];
			   }   
 		   Mout[i][j] = ( int ) ( ( pixout * scalef ) >> 12 );
	   }	
}



//-------------------------------------------------------------------
// Define some kernels
void
SetKernel5x5LoG( int kernel[MAXK][MAXK], int *size, int *scalef )
{
    kernel[0][0] = 0;  kernel[0][1] = 0; kernel[0][2] =   1; kernel[0][3] = 0; kernel[0][4] = 0;
    kernel[1][0] = 0;  kernel[1][1] = 1; kernel[1][2] =   2; kernel[1][3] = 1; kernel[1][4] = 0;
    kernel[2][0] = 1;  kernel[2][1] = 2; kernel[2][2] = -16; kernel[2][3] = 2; kernel[2][4] = 1;
    kernel[3][0] = 0;  kernel[3][1] = 1; kernel[3][2] =   2; kernel[3][3] = 1; kernel[3][4] = 0;
    kernel[4][0] = 0;  kernel[4][1] = 0; kernel[4][2] =   1; kernel[4][3] = 0; kernel[4][4] = 0;
	*scalef = 4096;
	*size = 5; // The kernel size 
}


int
SetKernel3x3Blur(  int kernel[MAXK][MAXK], int *size, int *scalef  )
{
    kernel[0][0] = 1;  kernel[0][1] =  1; kernel[0][2] =  1;
    kernel[1][0] = 1;  kernel[1][1] =  1; kernel[1][2] =  1;
    kernel[2][0] = 1;  kernel[2][1] =  1; kernel[2][2] =  1;
	*scalef = 4096 / 9;
	*size = 3;
}

void
SetKernel3x3LowPass(  int kernel[MAXK][MAXK], int *size, int *scalef  )
{
    kernel[0][0] = 1 ; kernel[0][1] = 2 ; kernel[0][2] =  1;
    kernel[1][0] = 2 ; kernel[1][1] = 4 ; kernel[1][2] =  2;
    kernel[2][0] = 1 ; kernel[2][1] = 2;  kernel[2][2] =  1;
	*scalef = 256;//4096 / 16;
	*size = 3;
}


int density[256];

void
MinMaxStretch(  int Min[NROWS][NCOLS],
                int Mout[NROWS][NCOLS],
				int cutoff) 
{
    int i, j, k;

    int minColor, maxColor;

    for (i = 0; i < NROWS; i++)
	{
        for (j = 0; j < NCOLS; j++)
		{
            density[ Min[i][j] ]++;
        }
    }

    int w = 0;
	int cutoffi = cutoff * ( ( NROWS * NCOLS ) >> 12 );
	
	minColor = 0;
    while (w < cutoffi)
	{
        w += density[minColor++];
    }
    w = 0;
	maxColor = 255;
    while (w < cutoffi)
	{
        w += density[maxColor--];
    }
	
	int colordiff = (maxColor - minColor);
	
    // Create the new image with this information.
    for (i = 0; i < NROWS; i++)
	{
        for (j = 0; j < NCOLS; j++)
		{
            // Set any pixel that has < minimum color equal to zero, and set
            // any pixel that has > maximum color equal to maxColor.
            if (Min[i][j] < minColor)
			{
                Mout[i][j] = 0;
            }
			else
			if (Min[i][j] > maxColor)
			{
                Mout[i][j] = 255;
            }
			else
			{
                // Create the resulting pixel based on a linear scaling factor
                // that takes into account the min and max colors.
                Mout[i][j] = ( ( Min[i][j] - minColor) * 255 ) / colordiff ;
            }
        }
    }
}





//-------------------------------------------------------------------
void
Binarize( 
          int Min[NROWS][NCOLS], 
		  int Mout[NROWS][NCOLS],
		  int threshold
		)
{
	int i, j;
	
	for(i=0; i < NROWS; i++ )
	   for(j=0; j < NCOLS ; j++ )
	   {
		   Mout[i][j] = Min[i][j] > threshold ? 255 : 0;
	   }
		
}


//-------------------------------------------------------------------
void
Erode( int Min[NROWS][NCOLS], 
       int Mout[NROWS][NCOLS],
	   int threshold
	 )
{
	int i, j, k, l;
	
	int pxaccum;
	
	int kernel_size = 3;

	for(i=0+kernel_size/2; i < NROWS - kernel_size/2; i++ )
	   for(j=0+kernel_size/2; j < NCOLS - kernel_size/2; j++ )
	   {
		  pxaccum = 0;
	      for( k=-kernel_size/2; k<=kernel_size/2; k++ )
		    for( l=-kernel_size/2; l<=kernel_size/2; l++ )
				pxaccum += Min[i+k][j+l];
		  pxaccum -= Min[i][j];
		  // If surrounding pixels average less than the threshold, erase pixel
		  if ( pxaccum < (kernel_size * kernel_size - 1) * threshold )
		    Mout[i][j] = 0;
		  else 
		    Mout[i][j] = Min[i][j];
	   }
		
}

//-------------------------------------------------------------------
#ifdef LOAD_FROM_FILE
void
WriteData( char *filename, 
           volatile int Mout[NROWS][NCOLS] )
{
	FILE *fpout;
	int i, j;
	
	if( ( fpout = fopen( filename, "w" ) ) == NULL )
	{
		printf("Can't create output file %s. Exiting...\n", filename );
		exit( -1 );
	}
		
	// Write matrix to text file:
	for(i=0; i<NROWS; i++)
	{
		for(j=0; j<NCOLS; j++)
		{
			fprintf(fpout,"%3d ", Mout[i][j] );
		}
		fprintf(fpout, "\n");
    }		
	fclose( fpout );
}
#endif



//-------------------------------------------------------------------
int
main( int argc, char **argv)
{
	int kernelsize;
    int scalef;

#ifdef LOAD_FROM_FILE
	if ( argc != 3 )
	{
		printf("Usage: aship infile outfile\n");
		return 0;
	}
	printf("Loading data...\n");
	LoadData( argv[1], Min );
#endif
	
	printf("Processing data...\n");
	
	// Lowpass with a 3x3 kernel:
	SetKernel3x3Blur( kernel, &kernelsize, &scalef );
	ConvolveImage( Min, Mb, kernel, kernelsize, scalef );
	ConvolveImage( Mb,  Ma, kernel, kernelsize, scalef );
#ifdef LOAD_FROM_FILE
    WriteData( "../data/imageblur.txt", Ma);
#endif	

    // Increae bright by 1.5X:
    ScaleImageAndSaturateToUint8( Ma, Mb, (int) (6144) ); // Scale factor is 1.5 (1.5 * 4096)
#ifdef LOAD_FROM_FILE
    WriteData( "../data/imagebright.txt", Mb);
#endif	
	
	// Contrast enhancement:
	MinMaxStretch( Mb, Ma, (int) ( 4096 * 0.3 ) ); // Contrast factor is 0.3
#ifdef LOAD_FROM_FILE
    WriteData( "../data/imagecontrast.txt", Ma);
#endif	

    // Edge detection:
    SetKernel5x5LoG( kernel, &kernelsize, &scalef );
	ConvolveImage( Ma, Mb, kernel, kernelsize, scalef ); 
    // Saturate to uint8:
	SaturateToUint8( Mb, Ma );
#ifdef LOAD_FROM_FILE
    WriteData( "../data/imageedgedet.txt", Ma);
#endif	

    // Binarize image: black if below 128, white otherwise
    Binarize( Ma, Mb, 128 );

	// Denoise image
#ifdef LOAD_FROM_FILE	
    Erode( Mb, Ma, 64 ); // higher = erode more
	printf("Writing data...\n");
	WriteData( argv[2], Ma );
#else
    Erode( Mb, Mout, 64 ); // higher = erode more
#endif
	
	return 1;
}
