CC = gcc
SOURCES = edge_detect.c clsystems.c motion_detect.c filters.c justcomp.c
CFLAGS = -Wall -g `pkg-config --cflags opencv` `pkg-config --libs opencv` -lm
BINS = motion
OBJS_LIB = $(SOURCES:.c=.o)

all:motion edge eyeget

motion: motion.c $(SOURCES)
	$(CC) $(CFLAGS) -o motion motion.c $(SOURCES)

edge: edge.c $(SOURCES)
	$(CC) $(CFLAGS) -o edge edge.c $(SOURCES)

eyeget: eyeget.c $(SOURCES)
	$(CC) $(CFLAGS) -o eyeget eyeget.c $(SOURCES)

fcompare: fcompare.c $(SOURCES)
	$(CC) $(CFLAGS) -o fcomp fcompare.c $(SOURCES)

clean:
	rm *.o

