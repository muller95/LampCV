#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include "justcomp.h"

int main(int argc, char **argv)
{
	int dev, flag;
	cvCapture *capt;
	IplImage *frame, *prev, *curr, *motion;

	dev = 0;
	if (argc > 1) {
		dev = atoi(argv[1]);
	}

	capt = cvCaptureFromCAM(dev);
	if (!capt) {
		perror("Error opening cam!");
		exit(1);
	}
	cvNamedWindow("Original", CV_WINDOW_NORMAL);

	cvSetCaptureProperty(capt,CV_CAP_PROP_FRAME_WIDTH, 160);
	cvSetCaptureProperty(capt, CV_CAP_PROP_FRAME_HEIGHT, 120);
	 
	flag = 1;
	
	frame = cvQueryFrame(capt);
	prev = cvCloneImage(frame);
	
	while (flag) {
		int key;
	
		frame = cvQueryFrame(capt);
		curr = cvCloneImage(frame);	

		motion = frapscompare(curr, prev);

		cvShowImage("Original", motion);

			cvReleaseImage(&prev);
		cvReleaseImage(&motion);
		prev = curr;
	}
	
	return 0;

}
