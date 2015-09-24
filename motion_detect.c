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
//	gaussblur(copy, 3.0);
	medianfilter(curr, 5);
	cvdata = getvals(curr);
	cvReleaseImage(&copy);

	copy = cvCloneImage(prev);	
//	gaussblur(copy, 3.0);
	medianfilter(curr, 5);
	
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
			if (deriv != 0 && d != 0) {
				cvLine(res, cvPoint(x, y), cvPoint(x + d, y),
					CV_RGB(0, 255, 0), 0, 8, 0);
			}
		}
	}
	
	return res;
//	return curr;
}
