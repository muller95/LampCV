#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include "clsystems.h"
#include "filters.h"

IplImage *frapscompare(IplImage *curr, IplImage *prev)
{
	int x, y, h, w;
	IplImage *res, *copy;
	unsigned char *pvdata, *cvdata;
	
	h = curr->height;
	w = curr->width;

	copy = cvCloneImage(curr);
	gaussblur(copy, 1.0);
	cvdata = getvals(curr);
	cvReleaseImage(&copy);

	copy = cvCloneImage(prev);	
	gaussblur(copy, 1.0);
	pvdata = getvals(prev);
	cvReleaseImage(&copy);
	
	res = cvCloneImage(prev);
	
	for(y = 0;y < h;y++){
		for(x = 0;x < w;x++){
			if(cvdata[y*w+x]!=pvdata[y*w+x]){
				res->imageData[res->nChannels * (y * w + x) + 0] = 0;
				res->imageData[res->nChannels * (y * w + x) + 1] = 255;
				res->imageData[res->nChannels * (y * w + x) + 2] = 0;
			}

		}
	}
	free(pvdata);
	free(cvdata);

	return res;		
}
