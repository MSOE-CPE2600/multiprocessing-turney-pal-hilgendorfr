/// 
//  mandel.c
//  Based on example code found here:
//  https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
//
//  Converted to use jpg instead of BMP and other minor changes
//
//  Name: Ryan Pal Hilgendorf
//  Assignment: Lab 11 & 12
//  Secton: CPE 2600 121
///
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>
#include "jpegrw.h"

// local routines
static int iteration_to_color( int i, int max );
static int iterations_at_point( double x, double y, int max );
static void compute_image( imgRawImage *img, double xmin, double xmax,
									double ymin, double ymax,
									int max, int threadCount );
void* threading(void* arg);
static void show_help();

typedef struct thread {
	imgRawImage* image;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	int max;
	int threadCount;
	pthread_t threadID;
} thread;

int main( int argc, char *argv[] )
{
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.
	const char *outfile = "mandel";
	double xcenter = 0;
	double ycenter = 0;
	double xscale = 4;
	double yscale = 0; // calc later
	int    image_width = 1000;
	int    image_height = 1000;
	int    max = 1000;
	int    childCount = 8;
	int    threadCount = 1;
	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:c:t:h"))!=-1) {
		switch(c) 
		{
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				xscale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'c':
			    childCount = atoi(optarg);
				break;
			case 't':
			    threadCount = atoi(optarg);
				if (threadCount > 20) {
					printf("Max thread count 20. Setting to max.");
					threadCount = 20;
				}
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}

	int imageCount = 50;
	// scale increase for each subsequent image. Scale is 2x by default
	double scaleInterval = xscale / imageCount;

	// fork and let children create images.
	for (int i = 0; i < childCount; i++) {
		if (fork() == 0) {
			// what image to start at
			int start = i * (imageCount / childCount);
			int end;
			// decides ending image
			if (i == childCount - 1) {
				end = imageCount;
			} else {
				end = start + (imageCount / childCount);
			}

			// modified mandel algorithm
		    for (int j = start; j < end; j++) {
				// calculated scale for each image
		    	double scale = xscale + j * scaleInterval;
		    	// Calculate y scale based on x scale (settable)
				// and image sizes in X and Y (settable)
		    	yscale = scale / image_width * image_height;

		    	// Create a raw image of the appropriate size.
	            imgRawImage* img = initRawImage(image_width,image_height);

	            // Fill it with a black
	            setImageCOLOR(img,0);

	            // Compute the Mandelbrot image
    	        compute_image(img,xcenter-scale/2,xcenter+scale/2, \
		    	ycenter-yscale/2,ycenter+yscale/2,max,threadCount);

	            // Save the image in the stated file.
	        	char output[100] = "";
	        	sprintf(output, "%s%02d.jpg", outfile, j+1);
	            storeJpegImageFile(img,output);

    	    	// Display the configuration of the image.
            	printf("mandel: x=%lf y=%lf xscale=%lf yscale=%1f max=%d \
				outfile=%s\n",xcenter,ycenter,scale,yscale,max,output);

	            // free the mallocs
	           freeRawImage(img);
		    }
			exit(1);
		}
	}

	// parent waits for children to finish
	int status;
	for (int i = 0; i < childCount; i++) {
		wait(&status);
	}
	printf("All Processes Finished.\n");
	return 0;
}




/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iter;
}


// computes individual rows for each Mandelbrot image.
void *threading(void *arg) {
	thread *data = (thread*) arg;
	imgRawImage *img = data->image;

	int width = img->width;
	int height = img->height;

	int threadRows = height / data->threadCount;
	int start = data->threadID * threadRows;
	int end;
	if (data->threadID == data->threadCount - 1) {
		end = height;
	} else {
		end = start + threadRows;
	}

	for(int j=start;j<end;j++) {
		for(int i=0;i<width;i++) {

			// Determine the point in x,y space for that pixel.
			double x = data->xmin + i*(data->xmax-data->xmin)/width;
			double y = data->ymin + j*(data->ymax-data->ymin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,data->max);

			// Set the pixel in the bitmap.
			setPixelCOLOR(img,i,j,iteration_to_color(iters,data->max));
		}
	}
	return NULL;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
Calculations moved to threading()
*/

void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max, int threadCount )
{	
	pthread_t threads[threadCount];
	// array is needed to avoid a "data race" caused by using the same structure of data.
	thread passthroughThread[threadCount];

	for (int i = 0; i < threadCount; i++) {
		passthroughThread[i].image = img;
		passthroughThread[i].xmin = xmin;
		passthroughThread[i].xmax = xmax;
		passthroughThread[i].ymin = ymin;
		passthroughThread[i].ymax = ymax;
		passthroughThread[i].max = max;
		passthroughThread[i].threadID = i;
		passthroughThread[i].threadCount = threadCount;
		pthread_create(&threads[i], NULL, threading, &passthroughThread[i]);
	}
	for (int i = 0; i < threadCount; i++) {
		pthread_join(threads[i], NULL);
	}
}




/*
Convert a iteration number to a color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/
int iteration_to_color( int iters, int max )
{
	int color = 0x0912FF*iters/(double)max;
	return color;
}


// Show help message
void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
	printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-c <count>  Set the number of children the program will have (default=8)\n");
	printf("-t <count>  Set the number of thread the program will have (default=1)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}
