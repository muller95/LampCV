#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include "clsystems.h"

IplImage *sobel(IplImage *frame);

IplImage *sobel(IplImage *frame)
{
	double thres = 50;
	int x,y,w,h, chans;
	IplImage *res;
	unsigned char *vdata;
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
			gx = vdata[(y+1)*w+x-1] + 2.0 * vdata[(y+1)*w+x] + vdata[(y + 1) * w + (x + 1)]
				- (vdata[(y - 1) * w + x - 1] + 2.0 * vdata[(y - 1) * w + x] + vdata[(y - 1) * w + x +1]);
			gy = vdata[(y - 1) * w + x + 1] + 2.0 * vdata[y * w + x + 1] + vdata[(y + 1) * w + x +1]
				- (vdata[(y - 1) * w + x - 1] + 2.0 * vdata[y * w + x - 1] + vdata[(y + 1) * w + x -1]);
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
