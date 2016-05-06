#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include "iplimage.h"
#include "iplvideo.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

enum iometh {
	IO_READ,
	IO_MMAP
};

static int xioctl(int fd, int request, void *arg)
{
	int r;

	do {
		r = ioctl(fd, request, arg);
	} while (r == -1 && errno == EINTR);

	return r;
}

static unsigned char *ioread(struct IplDev *dev)
{
	unsigned char *res;
 
	res = (unsigned char *)malloc(dev->length);
//	printf("size=%zd\n", dev->length);
	if (read(dev->fd, res, dev->length) == -1) {
		free(res); 
		return NULL;
	}
	return res;	
}

static int startcap(struct IplDev *dev)
{
	struct v4l2_buffer buf;
	enum v4l2_buf_type type;

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = 0;
	
	if (xioctl(dev->fd, VIDIOC_QBUF, &buf) == -1)
		return -1;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (xioctl(dev->fd, VIDIOC_STREAMON, &type) == -1)
		return -1;

	return 1;
}

static int readframe(struct IplDev *dev)
{
	struct v4l2_buffer buf;

	CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	
	if (xioctl(dev->fd, VIDIOC_DQBUF, &buf) == -1) {
		if (errno == EAGAIN)
			return 0;
		return -1;
	}
	
//	printf("used=%d length=%d\n", buf.bytesused, dev->length);
	if (xioctl(dev->fd, VIDIOC_QBUF, &buf) == -1)
		return -1;
	
	return 1;
}

static unsigned char *iommap(struct IplDev *dev)
{
	int rc;
//	unsigned char *res;
	
/*	res = NULL;
	res = mmap(NULL, dev->length, PROT_READ | PROT_WRITE, MAP_SHARED, dev->fd, dev->offset);
	
	if (res == MAP_FAILED) 
		return NULL;*/
	
	while ((rc = readframe(dev)) == 0);
	
	if (rc < 0) {
	//	free(res);
		fprintf(stderr, "could not read frame\n");
		return NULL;
	}
	
	return dev->data;
}

static unsigned char r_from_yuv(int y, int u, int v)
{
	int r = (int)(y + 1.402 * (v - 128.0));
	r = (r < 0)? 0 : r;
	r = (r > 255)? 255 : r;
	return r; 
}

static unsigned char g_from_yuv(int y, int u, int v)
{
	int g = (int)(y - 0.34414 * (u - 128.0) - 0.71414 * (v - 128.0));
	g = (g < 0)? 0 : g;
	g = (g > 255)? 255 : g;
	return g; 
}

static unsigned char b_from_yuv(int y, int u, int v)
{
	int b = (int)(y + 1.772 * (u - 128.0));
	b = (b < 0)? 0 : b;
	b = (b > 255)? 255 : b;
	return b; 
}

static struct IplImage *yuv422_to_rgb(struct IplDev *dev, unsigned char *data) 
{
	int i, j;
	struct IplImage *res;
	
	res = malloc(sizeof(struct IplImage));
	res->width = dev->width;
	res->height = dev->height;
	res->nchans = 3;
	res->data = (unsigned char *)malloc(sizeof(unsigned char) * res->width * res->height * 3);
	for (i = 0, j = 0; i < dev->length; i += 4, j += 6) {
		unsigned char y1, y2, u, v;
		unsigned char r1, g1, b1, r2, g2, b2;
		
		y1 = data[i];
		u = data[i + 1];
		y2 = data[i + 2];
		v = data[i + 3];

		r1 = r_from_yuv(y1, u, v);
		g1 = g_from_yuv(y1, u, v);
		b1 = b_from_yuv(y1, u, v);

		r2 = r_from_yuv(y2, u, v);
		g2 = g_from_yuv(y2, u, v);
		b2 = b_from_yuv(y2, u, v);

		res->data[j] = r1;
		res->data[j + 1] = g1;
		res->data[j + 2] = b1;

		res->data[j + 3] = r2;
		res->data[j + 4] = g2;
		res->data[j + 5] = b2;
	}
	return res; 
}

static struct IplImage *yuv422_to_gray(struct IplDev *dev, unsigned char *data) 
{
	int i, j;
	struct IplImage *res;
	
	res = malloc(sizeof(struct IplImage));
	res->width = dev->width;
	res->height = dev->height;
	res->nchans = 1;
	res->data = (unsigned char *)malloc(sizeof(unsigned char) * res->width * res->height);
	for (i = 0, j = 0; i < dev->length; i += 4, j += 2) {
		unsigned char y1, y2;
		
		y1 = data[i];
		y2 = data[i + 2];

		res->data[j] = y1;
		res->data[j + 1] = y2;
	}
	return res; 
}

struct IplImage *ipl_getframe(struct IplDev *dev)
{
	unsigned char *data;
	struct IplImage *img;
	
	if (dev->iometh == IO_READ) 
		data = ioread(dev);
	else if (dev->iometh == IO_MMAP) 
		data = iommap(dev);
	else 
		return NULL;
	
	if (!data)
		return NULL;

	if (dev->mode) {
		img = yuv422_to_rgb(dev, data);
		if (dev->force_scale && (img->width != dev->fwidth || img->height != dev->fheight)) {
			ipl_scaleimg(&img, dev->fwidth, dev->fheight);
		}
		return img;
	}
	else {
		img = yuv422_to_gray(dev, data);
		if (dev->force_scale && (img->width != dev->fwidth || img->height != dev->fheight))
			ipl_scaleimg(&img, dev->fwidth, dev->fheight);
		return img;
	}
}

static int setiometh(struct IplDev *dev, struct v4l2_capability *cap, char *devname)
{	
	if (!(cap->capabilities & V4L2_CAP_READWRITE)) {
		if (errno == EINVAL)
			fprintf(stderr, "%s does not support read i/o\n", devname);
	} else { 
		dev->iometh = IO_READ;
		return 1;
	}
	
	if (!(cap->capabilities & V4L2_CAP_STREAMING))
		fprintf(stderr, "%s does not support streaming\n", devname);
	else { 
		dev->iometh = IO_MMAP;
		struct v4l2_requestbuffers req;
		struct v4l2_buffer buf;

		CLEAR(req);
		req.count = 1;
		req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		req.memory = V4L2_MEMORY_MMAP;

		if (xioctl(dev->fd, VIDIOC_REQBUFS, &req)) {
			if (errno == EINVAL)
				fprintf(stderr, "does not support memory mapping\n");
			return -1;
		}
	
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = 0;
		if (xioctl(dev->fd, VIDIOC_QUERYBUF, &buf) == -1) {
			fprintf(stderr, "could not query buffer\n");
			return -1;
		}
	
	
		if (startcap(dev) < 0) {
			fprintf(stderr, "already started capturing\n");
			return -1;
		}

		dev->length = buf.length;
		dev->offset = buf.m.offset;

		return 1;	
	}
	return -1;
}

static int setupdev(struct IplDev *dev, int width, int height)
{
	struct stat st;
	struct v4l2_format fmt;
	struct v4l2_capability cap;
	char devname[64];

	bzero(devname, 64);
	sprintf(devname, "/dev/video%d", dev->id);

	if ((dev->fd = open(devname, O_RDWR | O_NONBLOCK, 0)) == -1) {
		fprintf(stderr, "can not open %s\n", devname);
		return -1;
	}

	if (fstat(dev->fd, &st) == -1) {
		fprintf(stderr, "can not identify %s\n", devname);
		return -1;
	}

	if (!S_ISCHR(st.st_mode)) {
		fprintf(stderr, "%s is not a device\n", devname);
		return -1;
	}
		

	if (xioctl(dev->fd, VIDIOC_QUERYCAP, &cap) == -1) { 
		fprintf(stderr, "%s is not V4L2 device\n", devname);
		close(dev->fd);
		return -1;
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		if (errno == EINVAL)
			fprintf(stderr, "not video device\n");
		close(dev->fd);
		return -1;
	}	


	CLEAR(fmt);	
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;	
	if (width > 0 && height > 0) {
		fmt.fmt.pix.width = width;
		fmt.fmt.pix.height = height;
		if (xioctl(dev->fd, VIDIOC_S_FMT, &fmt) == -1) {
			fprintf(stderr, "error on setting device format:%s\n", strerror(errno));
			close(dev->fd);
			return -1;
		}
		dev->width = fmt.fmt.pix.width;
		dev->height = fmt.fmt.pix.height;
		dev->fmt = fmt.fmt.pix.pixelformat;
	} else {
		if (xioctl(dev->fd, VIDIOC_G_FMT, &fmt) == -1) {
			fprintf(stderr, "error on getting pix format:%s\n", strerror(errno));
			close(dev->fd);
			return -1;
		}
		dev->width = dev->fwidth = fmt.fmt.pix.width;
		dev->height = dev->fheight = fmt.fmt.pix.height;
		dev->fmt = fmt.fmt.pix.pixelformat;
	}

	if (setiometh(dev, &cap, devname) < 0) {
		fprintf(stderr, "can not set io method\n");
		close(dev->fd);
		return -1;
	}
	

	dev->data = NULL;
	dev->data = mmap(NULL, dev->length, PROT_READ | PROT_WRITE, MAP_SHARED, dev->fd, dev->offset);
	
	if (dev->data == MAP_FAILED) { 
		fprintf(stderr, "Error on mapping: %s\n", strerror(errno));
		return -1;	
	}

	return 1;
}

int ipl_setparams(struct IplDev *dev, int width, int height, int force_scale)
{
	munmap(dev->data, dev->length);
	close(dev->fd);
		
	width = (width <= 0)? dev->width : width;
	height = (height <= 0)? dev->height : height;
	dev->fwidth = width;
	dev->fheight = height;
	dev->force_scale = force_scale;
	return setupdev(dev, width, height);
}

struct IplDev *ipl_opendev(int id, int mode)
{ 
	struct IplDev *dev;

	dev = (struct IplDev *)malloc(sizeof(struct IplDev));
	dev->id = id;
	dev->mode = mode;
	dev->force_scale = IPL_FORCE_SCALE_OFF;

	if (setupdev(dev, 0, 0) < 0) {
		free(dev);
		return NULL;
	}

	return dev;
}
