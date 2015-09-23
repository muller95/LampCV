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
//	gaussblur(copy, 0.7);
	cvdata = getvals(curr);
	cvReleaseImage(&copy);

	copy = cvCloneImage(prev);	
//	gaussblur(copy, 0.7);
	pvdata = getvals(prev);
	cvReleaseImage(&copy);

	res = cvCloneImage(curr);
	for (y = 0; y < h; y++) {
		for (x = 0; x < w - 1; x++) {
			double deriv = (double)(pvdata[y * w + x + 1] - pvdata[y * w + x]);
			double dfdt = (double)(pvdata[y * w + x] - cvdata[y * w + x]);     
			int d = (int)(trunc(dfdt / deriv) + 0.5);
			if (deriv != 0 && d != 0) {
				cvLine(res, cvPoint(x, y), cvPoint(x + d, y), CV_RGB(0, 255, 0), 0, 8, 0);
			}
			//printf("d=%d\n", d);
		}
	}
	
	gaussblur(curr, 1.0);
	gaussblur(curr, 1.0);
	return curr;
}
