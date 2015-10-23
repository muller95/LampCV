#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
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
	int dev, flag, theta, r, lnthres = 25;
	int w, h;
	double thres = 200.0;
	int arange, d, **lndata;
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

	cvSetCaptureProperty(capt,CV_CAP_PROP_FRAME_WIDTH, 320);
	cvSetCaptureProperty(capt, CV_CAP_PROP_FRAME_HEIGHT, 240); 
	
	frame = cvQueryFrame(capt);
	w = frame->width;
	h = frame->height;

	arange = 180;
	d = (int)sqrt(w * w + h * h);
//	cvReleaseImage(&frame);

	while (flag) {
		int key;
	
		frame = cvQueryFrame(capt);
		edge = sobel(frame, thres, 1.0);
		lndata = hough(frame, thres, 1.0);
		printf("HOUGH DONE\n");	

		for (theta = 0; theta < arange; theta++) {
			for (r = 0; r < d; r++) {
//				printf("theta=%d r=%d\n", theta, r);
				if (lndata[theta][r] > lnthres) {
	//				if (fabs(sin(theta)) > DBL_EPSILON && fabs(cos(theta)) > DBL_EPSILON) {
						int y1, y2;
						double th1;
						
						th1 = (double)(theta) * M_PI / 180.0;
						y1 = (int)((double)r / sin(th1));
						y2 = (int)(y1 - cos(th1) / sin(th1) * w);
						

				printf("(%d %d) (%d, %d)\n", 0, y1, w, y2);
						
			//			if (fabs(sin(th1)) > DBL_EPSILON && fabs(cos(th1)) > DBL_EPSILON)
							cvLine(edge, cvPoint(0, y1), cvPoint(w, y2), CV_RGB(255, 0, 0), 2, 8, 0);
	//				}
				}
			}
		}

		cvShowImage("Original", frame);
		cvShowImage("Edge", edge);

		switch ((key = cvWaitKey(5))) {
		case ESC:
			flag = 0;
			break;
		case KEY_PLUS:
			lnthres += 1;
			printf("lnthres=%d\n", lnthres);
			break;
		case KEY_MINUS:
			lnthres = (lnthres > 0.0)? lnthres - 1.0 : 0.0;
			printf("lnthres=%d\n", lnthres);
			break;
		default:
			if (key != -1)
				printf("keycode=%d\n", key);;
		}

//		cvReleaseImage(&frame);
	//	cvReleaseImage(&edge);
	}
	
	return 0;
}
