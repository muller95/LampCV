#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include "edge_detect.h"

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
	double thres = 50.0;
	CvCapture *capt;
	IplImage *frame, *edge;

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
	cvNamedWindow("Edge", CV_WINDOW_NORMAL);

	flag = 1;
	
	while (flag) {
		int key;
	
		frame = cvQueryFrame(capt);
		edge = sobel(frame, thres, 1.0);

		cvShowImage("Original", frame);
		cvShowImage("Edge", edge);

		switch ((key = cvWaitKey(5))) {
		case ESC:
			flag = 0;
			break;
		case KEY_PLUS:
			thres += 1;
			printf("thres=%lf\n", thres);
			break;
		case KEY_MINUS:
			thres = (thres > 0.0)? thres - 1.0 : 0.0;
			printf("thres=%lf\n", thres);
			break;
		default:
			if (key != -1)
				printf("keycode=%d\n", key);;
		}

	//	cvReleaseImage(&frame);
	//	cvReleaseImage(&edge);
	}
	
	return 0;
}
