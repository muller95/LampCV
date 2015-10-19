#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include "clsystems.h"
#include "filters.h"

static int corrind(int x, int y, int w, int h)
{
	int x1, y1;

	x1 = (x < 0) ? 0 : x;
	x1 = (x1 >= w)? w - 1 : x1;


	y1 = (y < 0) ? 0 : y;
	y1 = (y1 >= h)? h - 1 : y1;

	return y1 * w + x1;
}

static void grad(const unsigned char *vdata, int w, int h, int x, int y, double *gx, double *gy)
{
	*gx = vdata[corrind(x - 1, y + 1, w, h)] + 2.0 * vdata[corrind(x, y + 1, w, h)] + vdata[corrind(x + 1, y + 1, w, h)]
		- (vdata[corrind(x - 1, y - 1, w, h)] + 2.0 * vdata[corrind(x, y - 1, w, h)] + vdata[corrind(x + 1, y - 1, w, h)]);
	*gy = vdata[corrind(x + 1, y - 1, w, h)] + 2.0 * vdata[corrind(x + 1, y, w, h)] + vdata[corrind(x + 1, y + 1, w, h)]
		- (vdata[corrind(x - 1, y - 1, w, h)] + 2.0 * vdata[corrind(x - 1, y, w, h)] + vdata[corrind(x - 1, y + 1, w, h)]);

}
IplImage *sobel(IplImage *frame, double thres, double sigma)
{
	int x,y,w,h, chans;
	IplImage *res, *copy;
	unsigned char *vdata;

	copy = cvCloneImage(frame);
	gaussblur(copy, sigma);

	h = frame->height;
	w = frame->width;
	vdata = getvals(frame);
	res = cvCreateImage(cvSize(w, h), frame->depth, frame->nChannels);
	chans = frame->nChannels;
  
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			res->imageData[chans * (y * w + x) + 0] = 0;
			res->imageData[chans * (y * w + x) + 1] = 0;
			res->imageData[chans * (y * w + x) + 2] = 0;
		}
	}
  
	for(y = 1; y < h-1;y++){
		for(x = 1;x < w-1;x++) {
			double gx, gy, gr;
			
			grad(vdata, w, h, x, y, &gx, &gy);

/*			
			gx = vdata[(y+1)*w+x-1] + 2.0 * vdata[(y+1)*w+x] + vdata[(y + 1) * w + (x + 1)]
				- (vdata[(y - 1) * w + x - 1] + 2.0 * vdata[(y - 1) * w + x] + vdata[(y - 1) * w + x +1]);
			gy = vdata[(y - 1) * w + x + 1] + 2.0 * vdata[y * w + x + 1] + vdata[(y + 1) * w + x +1]
				- (vdata[(y - 1) * w + x - 1] + 2.0 * vdata[y * w + x - 1] + vdata[(y + 1) * w + x -1]);
*/
			
			
			gr = sqrt(gx * gx + gy * gy);
      
			if (gr >= thres) {
				res->imageData[chans * (y * w + x) + 0] = 255;
				res->imageData[chans * (y * w + x) + 1] = 255;
				res->imageData[chans * (y * w + x) + 2] = 255;
			}   
			
		}
	}
	free(vdata);
	return res;
}

double **hough(IplImage *frame, double thres, double sigma)
{
	int x, y, w, h, i;
	int d, arange;
	IplImage *copy;
	unsigned char *vdata;
	double **acc;

	h = frame->height;
	w = frame->width;

	arange = (int)(2.0 * M_PI * 100.0 + 2);
	acc = malloc(sizeof(double *) * arange);
	d = (int)sqrt(8 * (w * w + h * h)) + 2;
	for (i = 0; i < arange; i++)
		acc[i] = calloc(d, sizeof(double)); 

	copy = cvCloneImage(frame);
	gaussblur(copy, sigma);

	vdata = getvals(frame);

	for(y = 1; y < h-1;y++){
		for(x = 1;x < w-1;x++) {
			double gx, gy, gr;
			grad(vdata, w, h, x, y, &gx, &gy);
			
			gr = sqrt(gx * gx + gy * gy);
			
			if (gr >= thres) {
				int ang, p;
				ang = (int)(atan2(gy, gx) * 100.0 + arange / 2);
				p = (int)(cos(ang) * x + sin(ang) * y + d / 2);
				acc[ang][p] += 1;
			}
		}
	}

	free(vdata);
	return 0;
}
