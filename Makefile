IPLPATH=/home/vadim/LampCV-4-new/ipllib
MYXPATH=/home/vadim/LampCV-4-new/myxlib
CC = gcc
CFLAGS = -g -Wall -lm -lX11
SRCS =  $(IPLPATH)/iplimage.c $(IPLPATH)/iplvideo.c draw.c
SRCS += $(MYXPATH)/xfuncs.c 
SRCS += justcomp.c motion_detect.c clsystems.c

all:fcomp motion

fcomp: fcompare.c $(SRCS)
	$(CC) $(CFLAGS) -I/usr/include/libpng12 -lpng12 -o fcomp fcompare.c $(SRCS)

motion:	motion.c $(SRCS)
	$(CC) $(CFLAGS) -I/usr/include/libpng12 -lpng12 -o motion motion.c $(SRCS)
