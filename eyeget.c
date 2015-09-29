#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include "clsystems.h"

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

int eyeget(IplImage *frame, int thres, CvPoint *center)
{
	unsigned char *vdata, *mask;
	int x, y, w, h, ax, ay, n, chans;
	int xmax, xmin, ymax, ymin;
	int r;

	chans = frame->nChannels;
	vdata = getvals(frame);
	h = frame->height;
	w = frame->width;
	mask = calloc(w * h, sizeof(char));

	ymax = 0;
	ymin = h;
	xmax = 0;
	xmin = w;

	n = 0;
	ax = 0;
	ax = ay = 0;
	r = -1;
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			if (vdata[y * w + x] < thres) {
				frame->imageData[chans*(y*w+x)+0] = 255;
				frame->imageData[chans*(y*w+x)+1] = 0;
				frame->imageData[chans*(y*w+x)+2] = 0;
				n++;
				ax += x;
				ay += y;
				xmax = (x > xmax)? x : xmax;
				ymax = (y > ymax)? y : ymax;
				xmin = (x < xmin)? x : xmin;
				ymin = (y < ymin)? y : ymin;

			}
		}
	}

	if (n > 0) {	
		center->x = ax / n;
		center->y = ay / n;
		r = (xmax - xmin < ymax - ymin)? (xmax - xmin) / 2 : (ymax - ymin) / 2;
	}

	return r;
}


	

int main(int argc, char **argv)
{
	int dev, flag, delta = 3;
	int thres = 50, r, pr = -1;
	CvPoint center;
	CvCapture *capt;
	IplImage *frame;

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

	flag = 1;
	
	while (flag) {
		int key;
	
		frame = cvQueryFrame(capt);
		r = eyeget(frame, thres, &center);
		if (r > 0 && pr != -1) {
			if (r - pr > delta) {
				cvCircle(frame, center, r, CV_RGB(255, 0, 0), 2, 8, 0);
			} else {
				cvCircle(frame, center, r, CV_RGB(0, 255, 0), 2, 8, 0);
			}
		}
		pr = r;
		cvShowImage("Original", frame);

		switch ((key = cvWaitKey(5))) {
		case ESC:
			flag = 0;
			break;
		case KEY_PLUS:
			thres += 1;
			printf("thres=%d\n", thres);
			break;
		case KEY_MINUS:
			thres = (thres > 0)? thres - 1 : 0;
			printf("thres=%d\n", thres);
			break;
		case ARROW_UP:
			delta += 1;
			printf("delta=%d\n", thres);
			break;
		case ARROW_DOWN:
			thres = (delta > 0)? delta - 1 : 0;
			printf("thres=%d\n", delta);
			break;
		default:
			if (key != -1)
				printf("keycode=%d\n", key);;
		}

	}
	
	return 0;
}
