#ifndef _H_IPLIMAGE
#define _H_IPLIMAGE

struct IplImage {
	unsigned char *data;
	int width, height, nchans;
};

struct IplImage *ipl_cloneimg(struct IplImage *img);
struct IplImage *ipl_creatimg(int w, int h, int mode);
struct IplImage *ipl_readimg(char *path, int mode);
void ipl_scaleimg(struct IplImage **img, int nwidth, int nheight);
void ipl_freeimg(struct IplImage **img);
#endif
