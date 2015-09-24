#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cv.h>
#include <math.h>
#include <highgui.h>
#include "clsystems.h"

void winblur(IplImage *img);
void gaussblur(IplImage *img, double sigma);
void median_filter(IplImage *img, int win_size);

int sortfunc(const void *a, const void *b)
{
	return *((const uchar*) a) - *((const uchar *) b);
}

void fillwin(unsigned char *data, uchar *tmp_arr, uint frame_w, int chans,
	uint x, uint y, uint channel, uint win_size)
{
	int i, j;
	uint o = 0;	
	
	for (i = -((int)win_size/2); i <= ((int)win_size/2); i++)
		for (j = -((int)win_size/2); j <= ((int)win_size/2); j++)
			tmp_arr[o++] = data[chans * (frame_w * (y + i) + (x + j)) + channel];
}

void medianfilter(IplImage *img, int wsize)
{
	int w, h, chans;
	uchar *data, *tmparr;
	
	w = img->width;
	h = img->height;
	chans = img->nChannels;
	
	data = (uchar *)img->imageData;

	int wszsqr = wsize * wsize;
	
	tmparr = (uchar *)calloc(wszsqr * chans, sizeof(uchar));

	int c, y, x;

	int sidesz = (wsize - 1) / 2;
	for (c = 0; c < chans; c++)
		for (y = sidesz; y < h-sidesz; y++)
			for (x = sidesz; x < w-sidesz; x++) {	
				fillwin(data, tmparr, w, chans, x, y, c, wsize);

				qsort(tmparr, wszsqr, sizeof(uchar), sortfunc);

				data[chans * (w * y + x) + c] = tmparr[wszsqr/2];
			}

	free(tmparr);	
}

void winblur(IplImage *img)
{
	int x,y,w,h, i, chans;
	unsigned char *data;
	IplImage *copy;
	copy = cvCloneImage(img);
	h = img->height;
	w = img->width;
	data = (unsigned char *)copy->imageData;
	chans = img->nChannels;
  	
	for (y = 1; y < h - 1; y++) {
		for (x = 1; x < w - 1; x++) {
			for (i = 0; i < chans; i++) {
				unsigned int sum = 0;

				sum += data[chans * ((y - 1) * w + x - 1) + i];
				sum += data[chans * ((y - 1) * w + x) + i];
				sum += data[chans * ((y - 1) * w + x + 1) + i];

				sum += data[chans * (y * w + x - 1) + i];
				sum += data[chans * (y * w + x) + i];
				sum += data[chans * (y * w + x + 1) + i];

				sum += data[chans * ((y + 1) * w + x - 1) + i];
				sum += data[chans * ((y + 1) * w + x) + i];
				sum += data[chans * ((y + 1) * w + x + 1) + i];

				sum /= 9;
				img->imageData[chans * (y * w + x) + i] = sum;
			}
		}
	}
}

static double gaussfunc(double x, double sigma)
{
	return exp((-1) * (x * x) / (2.0 * sigma * sigma));
}


static void gblurx(unsigned char *data, double *tmp, int y, int x, int w, int h, int chans, double *ws, int sigma3)
{
	double s = 0.0;	
	int i, j;

// this summation must consider cases, when there are less pixels,
// than 2 * sigma3, i.e. borders of an image.

	for (i = 0; i <= 2 * sigma3; i++)
		s += ws[i];

	for (i = 0; i < chans; i++) {
		double c = 0.0;
	
		for (j = -sigma3; j <= sigma3; j++)	{
			int curx = x + j;
			curx = (curx < 0) ? 0 : curx;
			curx = (curx > (w - 1)) ? (w - 1) : curx;
		
			c += ws[j + sigma3] * (double)(data[chans * (w * y + curx) + i]);	
		}
		
		tmp[chans * (w * y + x) + i] = c / s;
	}

}

static void gblury(double *data, double *tmp, int y, int x, int w, int h, int chans, double *ws, int sigma3)
{
	double s = 0.0;	
	int i, j;

	for (i = 0; i <= 2 * sigma3; i++)
		s += ws[i];

	for (i = 0; i < chans; i++) {
		double c = 0.0;
		
		for (j = -sigma3; j <= sigma3; j++) {
			int cury = y + j;
			cury = (cury < 0) ? 0 : cury;
			cury = (cury > (h - 1)) ? (h - 1) : cury;
		
			c += ws[j + sigma3] * data[chans * (w * cury + x) + i];
		}
		
		tmp[chans * (w * y + x) + i] = c / s;
	}
}

void gaussblur(IplImage *img, double sigma)
{
	int x, y, i, sigma3;
	int w, h, chans;
	double *ws, *tmpx, *tmpy;
	unsigned char *data;
	
	w = img->width;
	h = img->height;
	sigma3 = (int)(3.0 * sigma);
	chans = img->nChannels;
	
	ws = (double *)malloc(sizeof(double) * (2 * sigma3 + 1));
	data = (unsigned char *)img->imageData;
	tmpx = (double *)calloc(w * h * chans, sizeof(double));
	tmpy = (double *)calloc(w * h * chans, sizeof(double));


// here is an idea for a better implementation: use one temporal buffer and
// after convolution with X part, store the result here. Then convolve with Y
// part, replacing original signal with obtained result.

	for (i = 0; i <= 2*sigma3; i++) {
		ws[i] = gaussfunc(i - sigma3, sigma);
	}

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			gblurx(data, tmpx, y, x, w, h, chans, ws, sigma3);
		}
	}

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			gblury(tmpx, tmpy, y, x, w, h, chans, ws, sigma3);
		}
	}

	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
			for (i = 0; i < chans; i++)
			{
				img->imageData[chans * (y * w + x) + i] 
					= (int)(tmpy[chans * (y * w + x) + i]);
			}

	
	free(ws);
	free(tmpx);
	free(tmpy);
}


