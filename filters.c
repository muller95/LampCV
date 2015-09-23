#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cv.h>
#include <math.h>
#include <highgui.h>
#include "clsystems.h"

void winblur(IplImage *img);
void gaussblur(IplImage *img, double sigma);

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
	return /*1.0 / sqrt(2.0 * M_PI * sigma * sigma) * */exp((-1) * (x * x) / (2.0 * sigma * sigma));
}


static void gblurx(unsigned char *data, double *tmp, int y, int x, int w, int h, int chans, double *ws, int sigma3)
{
	double c = 0.0;
	double s = 0.0;	
	int i, j;

	for (i = 0; i <= 2 * sigma3; i++) {
		s += ws[i];
	}

	for (i = 0;	i < chans; i++) {
		for (j = -sigma3; j <= sigma3; j++)	{
			int curx = x + j;
			curx = (curx < 0) ? 0 : curx;
			curx = (curx > (w - 1)) ? (w - 1) : curx;
			c += ws[j + sigma3] * (double)(data[chans * (w * y + curx) + i]);
		}
		tmp[chans * (w * y + x) + 1] = c / s;
	}
}

static void gblury(unsigned char *data, double *tmp, int y, int x, int w, int h, int chans, double *ws, int sigma3)
{
	double c = 0.0;
	double s = 0.0;	
	int i, j;

	for (i = 0; i <= 2 * sigma3; i++) {
		s += ws[i];
	}

	for (i = 0;	i < chans; i++) {
		for (j = -sigma3; j <= sigma3; j++)	{
			int cury = y + j;
			cury = (cury < 0) ? 0 : cury;
			cury = (cury > (h - 1)) ? (h - 1) : cury;
			c += ws[j + sigma3] * (double)(data[chans * (w * cury + x) + i]);
		}
		tmp[chans * (w * y + x) + 1] = c / s;
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


	
	for (i = 0; i <= 2*sigma3; ++i) {
		ws[i] = gaussfunc(i - sigma3, sigma);
	}

	for (y = 0; y < h; ++y) {
		for (x = 0; x < w; ++x) {
			gblurx(data, tmpx, y, x, w, h, chans, ws, sigma3);
		}
	}

	for (y = 0; y < h; ++y) {
		for (x = 0; x < w; ++x) {
			gblury(data, tmpy, y, x, w, h, chans, ws, sigma3);
		}
	}

	for (y = 0; y < h; ++y) {
		for (x = 0; x < w; ++x) {
			for (i = 0; i < chans; i++) {
				img->imageData[chans * (y * w + x) + i] = ((int)tmpx[chans * (y * w + x) + i] + tmpy[chans * (y * w + x) + i]);
			}
		}
	}

	
	free(ws);
	free(tmpx);
	free(tmpy);
}


