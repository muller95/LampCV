CC = gcc
SOURCES = main.c edge_detect.c clsystems.c motion_detect.c filters.c
CFLAGS = -Wall -g `pkg-config --cflags opencv` `pkg-config --libs opencv` -lm
BINS = motion
OBJS_LIB = $(SOURCES:.c=.o)


all: 
	$(CC) $(CFLAGS) -o $(BINS) $(SOURCES)


