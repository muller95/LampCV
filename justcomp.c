#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "ipllib/iplimage.h"
#include "ipllib/ipldefs.h"
#include "clsystems.h"
#include "myxlib/xfuncs.h"
#include "draw.h"

struct rct{
	int x1;
	int x2;
	int y1;
	int y2;
};

void getobj(unsigned char *cvdata, unsigned char *pvdata, int x, int y, int w, int h, struct rct *rectangle);
struct IplImage *framescompare(struct IplImage *curr, struct IplImage *prev)
{
	int x, y, h, w;
	struct IplImage *res;
	unsigned char *pvdata, *cvdata;
	struct rct rectangle;

	h = curr->height;
	w = curr->width;
	
	cvdata = getvals(curr);

	pvdata = getvals(prev);
	
	res = ipl_cloneimg(prev);
	
	for(y = 0; y < h; y++) {
		for(x = 0; x < w; x++) {
			if(abs(cvdata[y * w + x] - pvdata[y * w + x]) > 20) {
				rectangle.x1 = x;
				rectangle.y1 = y;
				rectangle.x2 = x;
				rectangle.y2 = y;
				getobj(cvdata, pvdata, x, y, w, h, &rectangle);
			}
			if((rectangle.x2 - rectangle.x1) * (rectangle.y2 - rectangle.y1) > 50) {
			drawRectangle(res, rectangle.x1, rectangle.y1, rectangle.x2, rectangle.y2);
			}
		}
	}
	
	free(pvdata);
	free(cvdata);
	return res;
}

void getobj(unsigned char *cvdata, unsigned char *pvdata, int x, int y, int w, int h, struct rct *rectangle)
{	
	cvdata[y * w + x] = 0;
	pvdata[y * w + x] = 0;
	rectangle->x1 = (x < rectangle->x1)? x : rectangle->x1;	
	rectangle->y1 = (y < rectangle->y1)? y : rectangle->y1;
	rectangle->x2 = (x > rectangle->x2)? x : rectangle->x2;	
	rectangle->y2 = (y > rectangle->y2)? y : rectangle->y2;
	
	if ((x + 1) < w && abs(cvdata[y * w + x + 1] - pvdata[y * w + x + 1]) > 20){ // right
		getobj(cvdata, pvdata, x + 1, y, w, h, rectangle);
	}if((x- 1) >= 0 && abs(cvdata[y * w + x - 1] - pvdata[y * w + x - 1]) > 20){ // left
		getobj(cvdata, pvdata, x - 1, y, w, h, rectangle);
	}if((y + 1) < h && abs(cvdata[(y + 1) * w + x] - pvdata[(y + 1) * w + x]) > 20){ // up
		getobj(cvdata, pvdata, x, y + 1, w, h, rectangle);
	}if((y - 1) >= 0 && abs(cvdata[(y - 1) * w + x] - pvdata[(y - 1) * w + x]) > 20){ // down
		getobj(cvdata, pvdata, x, y - 1, w, h, rectangle);
	}	
}

