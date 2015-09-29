#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include "clsystems.h"
#include "filters.h"

IplImage *lukkan1d(IplImage *curr, IplImage *prev);

IplImage *lukkan1d(IplImage *curr, IplImage *prev)
{
	int x,y,w,h;
	IplImage *res, *copy;
	unsigned char *pvdata, *cvdata;
	h = curr->height;
	w = curr->width;

	copy = cvCloneImage(curr);
	gaussblur(copy, 3.0);
//	medianfilter(curr, 3);
	cvdata = getvals(curr);
	cvReleaseImage(&copy);

	copy = cvCloneImage(prev);	
	gaussblur(copy, 3.0);
//	medianfilter(curr, 3);
	
	pvdata = getvals(prev);
	cvReleaseImage(&copy);

	res = cvCloneImage(curr);
	for (y = 0; y < h; y++) {
		for (x = 0; x < w - 1; x++) {
			double deriv = (double)(pvdata[y * w + x + 1] - pvdata[y * w + x]);
			double dfdt = (double)(pvdata[y * w + x] - cvdata[y * w + x]);     
			int d = (int)(trunc(dfdt / deriv) + 0.5);

			// filters are lagging like a shit
			// but i tried to set up a threshold here,
			// you should try it too.
			//
			//                  ||
			//                 _||_
			//                 \  /
			//                  \/
			//
			if (deriv != 0 && d != 0 && d > 25) {
				cvLine(res, cvPoint(x, y), cvPoint(x + d, y),
					CV_RGB(0, 255, 0), 0, 8, 0);
			}
		}
	}
	
	return res;
//	return curr;
}

void nearpix(unsigned int *mask, unsigned int w, unsigned int h)
{
	int y, x;

	for (y = 1; y <= h; y++) {
		for (x = 1; x <= w; x++) {
			uint nbs = 0;

			nbs += mask[(w + 2) * y + x + 1];
			nbs += mask[(w + 2) * y + x - 1];
			nbs += mask[(w + 2) * (y - 1) + x];
			nbs += mask[(w + 2) * (y + 1) + x];

			nbs += mask[(w + 2) * (y + 1) + x + 1];
			nbs += mask[(w + 2) * (y + 1) + x - 1];
			nbs += mask[(w + 2) * (y - 1) + x + 1];
			nbs += mask[(w + 2) * (y - 1) + x - 1];

			if (nbs < 4)
				mask[(w + 2) * y + x] = 0;
			if (nbs > 5)
				mask[(w + 2) * y + x] = 1;
		}
	}
}


IplImage *hornschunk(IplImage *curr, IplImage *prev, double alpha, int n)
{
	int x, y, w, h, i;
	double ix, iy, it; 
	double *un1, *vn1, aun, avn, *un, *vn;
	IplImage *res, *copy;
	unsigned char *pvdata, *cvdata;
	
	h = curr->height;
	w = curr->width;

	copy = cvCloneImage(curr);
	gaussblur(copy, 1.0);
//	medianfilter(curr, 3);
	cvdata = getvals(curr);
	cvReleaseImage(&copy);

	copy = cvCloneImage(prev);	
	gaussblur(copy, 1.0);
//	medianfilter(curr, 3);
	pvdata = getvals(prev);
	cvReleaseImage(&copy);
	
	res = cvCloneImage(prev);
	
	un = calloc(curr->width * curr->height, sizeof(double));
	vn = calloc(curr->width * curr->height, sizeof(double));
	
	un1 = calloc(curr->width * curr->height, sizeof(double));
	vn1 = calloc(curr->width * curr->height, sizeof(double));
	for (i = 0; i < n; i++) {
		for (y = 1; y < h - 1; y++) {
			for (x = 1; x < w - 1; x++) {
				ix = (double)(pvdata[(y+1)*w+x] - pvdata[y*w+x] + pvdata[(y+1)*w+(x+1)] - pvdata[y*w+(x+1)] 
						+ cvdata[(y+1)*w+x] - cvdata[y*w+x] + cvdata[(y+1)*w+(x+1)]-cvdata[y*w+(x+1)]);
				iy = (double)(pvdata[y*w+(x+1)] - pvdata[y*w+x] + pvdata[(y+1)*w+(x+1)] - pvdata[(y+1)*w+x] 
						+ cvdata[y*w+(x+1)] - cvdata[y*w+x] + cvdata[(y+1)*w+(x+1)]-cvdata[y*w+x]);
				it = (double)(cvdata[y*w+(x+1)] - pvdata[y*w+x] + cvdata[y*w+(x+1)] - pvdata[y*w+(x+1)] 
						+ cvdata[(y+1)*w+x] - pvdata[(y+1)*w+x] + cvdata[(y+1)*w+(x+1)]-cvdata[(y+1)*w+(x+1)]);
			
				aun = 1 / 6.0 * (double)(un[(y-1)*w+x] + un[(y+1)*w+x] + un[y*w+(x-1)] + un[y*w+(x+1)]) + 
						1/12.0 * (double)(un[(y-1)*w+(x-1)] + un[(y+1)*w+(x-1)] + un[(y+1)*w+(x-1)] + un[(y+1)*w+(x+1)]);
				avn = 1 / 6.0 * (double)(vn[(y-1)*w+x] + vn[(y+1)*w+x] + vn[y*w+(x-1)] + vn[y*w+(x+1)]) 
						+ 1/12.0 * (double)(vn[(y-1)*w+(x-1)] + vn[(y+1)*w+(x-1)] + vn[(y+1)*w+(x-1)] + vn[(y+1)*w+(x+1)]);

				un1[y * w + x] = aun - ix * (ix * aun + iy * avn + it) / (ix * ix + iy * iy + alpha * alpha);
				vn1[y * w + x] = avn - iy * (ix * aun + iy * avn + it) / (ix * ix + iy * iy + alpha * alpha);	  
			}
		}
	
		un = un1;
		vn = vn1;
	}


	
	for (y = 1; y < h - 1; y++) {
		for (x = 1; x < w - 1; x++) {
			if (fabs(un1[y*w+x]) > 10.0 || fabs(vn1[y*w+x]) > 10.0) { 
				//cvLine(res, cvPoint(x, y), cvPoint(x + un1[y*w+x], y + vn1[y*w+x]), CV_RGB(0, 255, 0), 0, 8, 0);
				res->imageData[res->nChannels * (y * w + x) + 0] = 0;
				res->imageData[res->nChannels * (y * w + x) + 1] = 255;
				res->imageData[res->nChannels * (y * w + x) + 2] = 0;
			}	
		}
	}

//	free(un);
//	free(vn);
	free(un1);
	free(vn1);
	free(pvdata);
	free(cvdata);
	return res;
}
