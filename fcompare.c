#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <string.h>
#include <png.h>
#include "ipllib/iplimage.h"
#include "ipllib/iplvideo.h"
#include "ipllib/ipldefs.h"

#include "justcomp.h"

#include "myxlib/xfuncs.h"

extern Display *theDisplay;

int camera_shoot(struct IplImage *img, XImage *ximg, GC theGC, Window theWindow, int x1, int y1)
{
	int x, y;
	
	for (y = 0; y < img->height; y++)
		for (x = 0; x < img->width; x++) {
			unsigned char r, g, b;
			r = img->data[img->nchans * (y * img->width + x) + 0];
			g = img->data[img->nchans * (y * img->width + x) + 1];
			b = img->data[img->nchans * (y * img->width + x) + 2];
			XPutPixel(ximg, x, y, ((r << 16) | (g << 8) | b));
		}
	XPutImage(theDisplay, theWindow, theGC, ximg, 0, 0, x1, y1, 640, 480);

	return 0;	
}

int main(int argc, char **argv)
{
	int x, y;

	Window win1;
	GC gc1;
	XEvent event;
	XImage *ximg1;

	int flag;
	struct IplImage *prev1, *curr1, *motion;
	struct IplDev *dev1;

	time_t start, end;
	int f;

	f = 0;
	start = 0;
	end = 0;
	
	x = 0;
	y = 0;
	initX();
	getXinfo();
	win1 = openWindow(500, 500, 640, 480, 0, &gc1);
	XNextEvent(theDisplay, &event);
	ximg1 = XGetImage(theDisplay, win1, x, y, 640, 480, AllPlanes, ZPixmap);
	if(XInitImage(ximg1) == 0) {
		fprintf(stderr,"error: XInitImage\n");
		return 1;
	}

	printf("first xgetimage done\n");

	if ((dev1 = ipl_opendev(0, IPL_RGB_MODE)) == NULL) {
		printf("error while creating device 0\n");
		return 1;
	}

	if (ipl_setparams(dev1, 320, 240, IPL_FORCE_SCALE_ON) < 0) {
		fprintf(stderr, "error on changing cam params\n");
		free(dev1);
		return 1;
	}
	if ((prev1 = ipl_getframe(dev1)) == NULL) {
		printf("error capturing prev1\n");
		return 1;
	}


	flag = 1;
	start = time(NULL);
	
	while (flag) {
		f++;	
		end = time(NULL);
		if (end - start >= 1.0) {
			printf("fps = %i\n", f);
			f = 0;
			start = end;
		}
	
		if ((curr1 = ipl_getframe(dev1)) == NULL) {
			printf("error capturing curr1\n");
			return 1;
		}
		motion = framescompare(curr1, prev1);
		ipl_scaleimg(&motion, 640, 480);
		camera_shoot(motion, ximg1, gc1, win1, x, y);
		ipl_freeimg(&prev1);
		prev1 = curr1;


		ipl_freeimg(&motion);
		XCheckTypedWindowEvent(theDisplay, win1, ButtonPress, &event);
		if (event.xbutton.button == 1)
			flag = 0;
	}

	ipl_freeimg(&curr1);
	XDestroyImage(ximg1);
	XDestroyWindow(theDisplay, win1);

	quitX();


	return 0;
}
