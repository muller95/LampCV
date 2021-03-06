#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ipllib/ipldefs.h"
#include "ipllib/iplimage.h"


unsigned char *getvals(struct IplImage *img) 
{
	int x, y, chans, w, h;
	unsigned char *data, *out;

	h = img->height;
	w = img->width;
	chans = img->nchans;
	out = malloc(sizeof(unsigned char) * w * h);
	data = (unsigned char *)img->data;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			int r, g, b, v;

			b = data[chans * (y * w + x) + 0];
			g = data[chans * (y * w + x) + 1];
			r = data[chans * (y * w + x) + 2];
	
			v = b;
			v = (g > v)? g : v;
			v = (r > v)? r : v;
			
			out[y * w + x] = v;
		}
	}

	return out;
}
