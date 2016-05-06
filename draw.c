#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "ipllib/iplimage.h"

void drawLine(struct IplImage *img, int x1, int y1, int x2, int y2)
{
	double tstep, t, len;
	int x, y, dx, dy;
	unsigned char red, green;
	
	dx = abs(x2-x1);
	dy = abs(y2-y1);

	len = sqrt(dx * dx + dy * dy);
	red = (len * 255) / 30;
	green = 255 - red;

	tstep = 1.0 / len; 

	for (t = 0.0; t <= 1.0; t += tstep) {
		x = (1 - t) * x1 + t * x2;
		y = (1 - t) * y1 + t * y2;

		if (x < 0 || x >= img->width || y < 0 || y >= img->height)
			continue;


		img->data[img->nchans * (y * img->width + x) + 0] = red;
		img->data[img->nchans * (y * img->width + x) + 1] = green;
		img->data[img->nchans * (y * img->width + x) + 2] = 0;
	}

}

void drawRectangle(struct IplImage *img, int x, int y, int x2, int y2)
{
	int x1, y1;
	unsigned char r, g, b;
/*	r = rand() % 255;
	g = rand() % 255;
	b = rand() % 255; */
	r = 128;
	g = 128;
	b = 0;
//	printf("(%d %d) (%d %d)\n", x, y, x2, y2);
	x1 = x;
	for (y1 = y; y1 < y2; y1++) {
		img->data[img->nchans * (y1 * img->width + x1) + 0] = r;
		img->data[img->nchans * (y1 * img->width + x1) + 1] = g;
		img->data[img->nchans * (y1 * img->width + x1) + 2] = b;
	}
	for (x1 = x; x1 < x2; x1++) {
		img->data[img->nchans * (y1 * img->width + x1) + 0] = r;
		img->data[img->nchans * (y1 * img->width + x1) + 1] = g;
		img->data[img->nchans * (y1 * img->width + x1) + 2] = b;
	}
	for (y1 = y; y1 < y2; y1++) {
		img->data[img->nchans * (y1 * img->width + x1) + 0] = r;
		img->data[img->nchans * (y1 * img->width + x1) + 1] = g;
		img->data[img->nchans * (y1 * img->width + x1) + 2] = b;
	}
	y1 = y;
	for (x1 = x; x1 < x2; x1++) {
		img->data[img->nchans * (y1 * img->width + x1) + 0] = r;
		img->data[img->nchans * (y1 * img->width + x1) + 1] = g;
		img->data[img->nchans * (y1 * img->width + x1) + 2] = b;
	}


}
