#ifndef _H_IPLVIDEO
#define _H_IPLVIDEO

#define IPL_DEVPARAM_WIDTH 0
#define IPL_DEVPARAM_HEIGHT 1
#define IPL_DEVPARAM_FORCE_SCALE 2

#define IPL_FORCE_SCALE_OFF 0
#define IPL_FORCE_SCALE_ON 1

struct IplDev {
	int id, fd;
	int width, height, fwidth, fheight;
	int fmt, mode, iometh, force_scale;
	unsigned int length, offset;
	unsigned char *data;
};

struct IplDev *ipl_opendev(int id, int mode);
struct IplImage *ipl_getframe(struct IplDev *dev);
int ipl_setparams(struct IplDev *dev, int width, int height, int force_scale);
#endif
