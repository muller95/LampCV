#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "ipllib/iplimage.h"
#include "ipllib/ipldefs.h"
#include "clsystems.h"
#include "myxlib/xfuncs.h"
#include "draw.h"

void nearpix(unsigned int *mask, unsigned int w, unsigned int h)
{
	int y, x;

	for (y = 1; y <= h; y++) {
		for (x = 1; x <= w; x++) {
			uint nbs = 0;

			nbs += mask[(w + 2) * y + x + 1];
			nbs += mask[(w + 2) * y + x - 1];
			nbs += mask[(w + 2) * (y - 1) + x];
			nbs += mask[(w + 2) * (y + 1) + x];

			nbs += mask[(w + 2) * (y + 1) + x + 1];
			nbs += mask[(w + 2) * (y + 1) + x - 1];
			nbs += mask[(w + 2) * (y - 1) + x + 1];
			nbs += mask[(w + 2) * (y - 1) + x - 1];

			if (nbs < 4)
				mask[(w + 2) * y + x] = 0;
			if (nbs > 5)
				mask[(w + 2) * y + x] = 1;
		}
	}
}


struct IplImage *hornschunk(struct IplImage *curr, struct IplImage *prev, double alpha, int n)
{
	int x, y, w, h, i;
	double ix, iy, it; 
	double *un1, *vn1, aun, avn, *un, *vn;
	double len;
	struct IplImage *res;
	unsigned char *pvdata, *cvdata;
	
	h = curr->height;
	w = curr->width;

	cvdata = getvals(curr);

	pvdata = getvals(prev);
	
	res = ipl_cloneimg(prev);
	
	un = calloc(curr->width * curr->height, sizeof(double));
	vn = calloc(curr->width * curr->height, sizeof(double));
	
	un1 = calloc(curr->width * curr->height, sizeof(double));
	vn1 = calloc(curr->width * curr->height, sizeof(double));
	for (i = 0; i < n; i++) {
		for (y = 1; y < h - 1; y++) {
			for (x = 1; x < w - 1; x++) {
				ix = (double)(1 / 4.0 * (pvdata[(y+1)*w+x] - pvdata[y*w+x] + pvdata[(y+1)*w+(x+1)] - pvdata[y*w+(x+1)] 
						+ cvdata[(y+1)*w+x] - cvdata[y*w+x] + cvdata[(y+1)*w+(x+1)]-cvdata[y*w+(x+1)]));
				iy = (double)(1 / 4.0 * (pvdata[y*w+(x+1)] - pvdata[y*w+x] + pvdata[(y+1)*w+(x+1)] - pvdata[(y+1)*w+x] 
						+ cvdata[y*w+(x+1)] - cvdata[y*w+x] + cvdata[(y+1)*w+(x+1)]-cvdata[y*w+x]));
				it = (double)(1 / 4.0 * (cvdata[y*w+(x+1)] - pvdata[y*w+x] + cvdata[y*w+(x+1)] - pvdata[y*w+(x+1)] 
						+ cvdata[(y+1)*w+x] - pvdata[(y+1)*w+x] + cvdata[(y+1)*w+(x+1)]-cvdata[(y+1)*w+(x+1)]));
			
				aun = 1 / 6.0 * (double)(un[(y-1)*w+x] + un[(y+1)*w+x] + un[y*w+(x-1)] + un[y*w+(x+1)]) + 
						1/12.0 * (double)(un[(y-1)*w+(x-1)] + un[(y+1)*w+(x-1)] + un[(y+1)*w+(x-1)] + un[(y+1)*w+(x+1)]);
				avn = 1 / 6.0 * (double)(vn[(y-1)*w+x] + vn[(y+1)*w+x] + vn[y*w+(x-1)] + vn[y*w+(x+1)]) 
						+ 1/12.0 * (double)(vn[(y-1)*w+(x-1)] + vn[(y+1)*w+(x-1)] + vn[(y+1)*w+(x-1)] + vn[(y+1)*w+(x+1)]);

				un1[y * w + x] = aun - ix * (ix * aun + iy * avn + it) / (ix * ix + iy * iy + alpha * alpha);
				vn1[y * w + x] = avn - iy * (ix * aun + iy * avn + it) / (ix * ix + iy * iy + alpha * alpha);	  
			}
		}
		un = un1;
		vn = vn1;
	}


	
	for (y = 1; y < h - 1; y++) {
		for (x = 1; x < w - 1; x++) {
			len = sqrt(fabs(un1[y*w+x]) * fabs(un1[y*w+x]) + fabs(vn1[y*w+x]) * fabs(vn1[y*w+x]));
			if (len > 5.0){
				drawLine(res, (int)x, (int)y, (int)(x + un1[y*w+x]), (int)(y + vn1[y*w+x]));
			}	
		}
	}

//	free(un);
//	free(vn);
	free(un1);
	free(vn1);
	free(pvdata);
	free(cvdata);
	return res;
}
