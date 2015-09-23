#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include "motion_detect.h"

enum KEY {	
	ARROW_LEFT	= 65361,
	ARROW_UP	= 65362,
	ARROW_RIGHT	= 65363,
	ARROW_DOWN 	= 65364,
	ESC		= 27,
	SPACE		= 32,
	KEY_P		= 112,
	KEY_PLUS	= 61,
	KEY_MINUS	= 45
};

int main(int argc, char **argv)
{
	int dev, flag;
	CvCapture *capt;
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
	cvNamedWindow("Original", CV_WINDOW_AUTOSIZE);

	flag = 1;
	
	frame = cvQueryFrame(capt);
	prev = cvCloneImage(frame);	
	while (flag) {
		int key;
	
		frame = cvQueryFrame(capt);
		curr = cvCloneImage(frame);	
		
		motion = lukkan1d(curr, prev);

		cvShowImage("Original", motion);

		switch ((key = cvWaitKey(10))) {
		case ESC:
			flag = 0;
			break;
		default:
			if (key != -1)
				printf("keycode=%d\n", key);;
		}

//		cvReleaseImage(&frame);
	//	cvReleaseImage(&prev);
		prev = curr;
	}
	
	return 0;
}
