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

IplImage *hornschunk(IplImage *curr, IplImage *prev, double *un, double *vn, double alpha)
{
	int x, y, w, h;
	double p1, p2, ix, iy, it, un1, vn1;
	IplImage *res, *copy;
	unsigned char *pvdata, *cvdata;
	h = curr->height;
	w = curr->width;

	copy = cvCloneImage(curr);
	gaussblur(copy, 5.0);
//	medianfilter(curr, 3);
	cvdata = getvals(curr);
	cvReleaseImage(&copy);

	copy = cvCloneImage(prev);	
	gaussblur(copy, 5.0);
//	medianfilter(curr, 3);
	pvdata = getvals(prev);
	cvReleaseImage(&copy);
	
	res = cvCloneImage(curr);
	for (y = 0; y < h - 1; y++) {
		for (x = 0; x < w - 1; x++) {
			p1 = pvdata[y * w + x];
			p2 = pvdata[y * w + x + 1];
			ix = p2 - p1;
			p2 = pvdata[(y + 1) * w + x];
			iy = p2 - p1;
	  
			p2 = cvdata[y * w + x];
			p1 = pvdata[y * w + x];
			it = p2 - p1;
	  
			un1 = un[y * w + x]  - ix * (ix * un[y * w + x] + iy * vn[y * w +x] + it) / (ix * ix + iy * iy + alpha * alpha);
			vn1 = vn[y * w + x]  - iy * (ix * un[y * w + x] + iy * vn[y * w +x] + it) / (ix * ix + iy * iy + alpha * alpha);
	  
			if (fabs(un1) > 10.0 || fabs(vn1) > 10.0) { 
				cvLine(res, cvPoint(x, y), cvPoint(x + un1, y + vn1), CV_RGB(0, 255, 0), 0, 8, 0);
				/*res->imageData[res->nChannels * (y * w + x) + 0] = 0;
				res->imageData[res->nChannels * (y * w + x) + 1] = 255;
				res->imageData[res->nChannels * (y * w + x) + 2] = 0;*/
			}

			un[y * w + x] = un1; 
			vn[y * w + x] = vn1; 
		}
	}

	return res;
}
