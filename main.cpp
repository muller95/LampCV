#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "k_means.h"

using namespace cv;
using namespace std;

#define QUAD_EDGE 50

enum KEY
{
	KEY_ARROW_LEFT	= 65361,
	KEY_ARROW_UP	= 65362,
	KEY_ARROW_RIGHT	= 65363,
	KEY_ARROW_DOWN	= 65364,
	KEY_ESC		= 27,
	KEY_SPACE	= 32
};

struct color
{
	color(uchar cr, uchar cg, uchar cb) : r(cr), g(cg), b(cb) {}

	uchar b;
	uchar g;
	uchar r;
};

void mouse_click(int e, int x, int y, int, void *arg)
{

}

double color_dist(struct color *c0, struct color *c1)
{
	return sqrt(pow(c0->b - c1->b, 2.0)
		+ pow(c0->g - c1->g, 2.0)
		+ pow(c0->r - c1->r, 2.0));
}

void clust(const Mat *in, Mat *out, uint k)
{
	uchar *hsv = (uchar *) malloc(sizeof(uchar) * in->cols * in->rows);
	uchar *means;
	uint *idx;
	uint w = in->cols;
	uint h = in->rows;

	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++) {
			uchar b = in->data[in->channels() * (w * y + x) + 0];
			uchar g = in->data[in->channels() * (w * y + x) + 1];
			uchar r = in->data[in->channels() * (w * y + x) + 2];
	
			uchar max = b;
			max = (g > max) ? g : max;
			max = (r > max) ? r : max;
	
			hsv[y*w+x] = max;	
		}
	
	idx = k_means(hsv, w*h, k, &means);

	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++) {
			out->data[out->channels() * (w * y + x) + 0]
				= means[idx[y * w + x]];
			out->data[out->channels() * (w * y + x) + 1]
				= means[idx[y * w + x]];
			out->data[out->channels() * (w * y + x) + 2]
				= means[idx[y * w + x]];
		}

	free(means);
	means = NULL;
	free(idx);
	idx = NULL;
	free(hsv);
	hsv = NULL;
}

uint *create_mask(const Mat *frame,
	const vector<Mat> *prev_frames, float coldist)
{
	uint w = frame->cols;
	uint h = frame->rows;
	const Mat *bg = &(*prev_frames)[0];	
	// mask indices start from 1, all elements, that have 0 and h + 1 as
	// one of their indices, are filled with 0
	uint *mask = (uint *) calloc((w + 2) * (h + 2), sizeof(uint));


	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			color c0 (frame->data[frame->channels() * (w * y + x) + 0],
				frame->data[frame->channels() * (w * y + x) + 1],
				frame->data[frame->channels() * (w * y + x) + 2]);
			color c1 (bg->data[bg->channels() * (w * y + x) + 0],
				bg->data[bg->channels() * (w * y + x) + 1],
				bg->data[bg->channels() * (w * y + x) + 2]);

			if (color_dist(&c0, &c1) > coldist)
				mask[(w + 2) * (y + 1) + x + 1] = 1;	
			else
				mask[(w + 2) * (y + 1) + x + 1] = 0;
		}
	}

	return mask;
}

void nearest_pixels(uint *mask, uint w, uint h)
{
	for (int y = 1; y <= h; y++) {
		for (int x = 1; x <= w; x++) {
			uint neightbours = 0;

			neightbours += mask[(w + 2) * y + x + 1];
			neightbours += mask[(w + 2) * y + x - 1];
			neightbours += mask[(w + 2) * (y - 1) + x];
			neightbours += mask[(w + 2) * (y + 1) + x];

			neightbours += mask[(w + 2) * (y + 1) + x + 1];
			neightbours += mask[(w + 2) * (y + 1) + x - 1];
			neightbours += mask[(w + 2) * (y - 1) + x + 1];
			neightbours += mask[(w + 2) * (y - 1) + x - 1];

			if (neightbours < 4)
				mask[(w + 2) * y + x] = 0;
			if (neightbours > 5)
				mask[(w + 2) * y + x] = 1;
		}
	}
}

uint *mask_to_quads(uint *mask, uint w, uint h, uint *sqrw, uint *sqrh)
{
	*sqrw = w / QUAD_EDGE + ((w % QUAD_EDGE) ? 1 : 0);
	*sqrh = h / QUAD_EDGE + ((h % QUAD_EDGE) ? 1 : 0);
	uint *quads = (uint *) calloc(*sqrw * *sqrh, sizeof(uint));
	for (int y = 1; y <= h; y++) {
		for (int x = 1; x <= w; x++) {
			uint sqrx = (x - 1) / QUAD_EDGE;
			uint sqry = (y - 1) / QUAD_EDGE;
		
			quads[*sqrw * sqry + sqrx] += mask[(w + 2) * y + x];
		}
	}

	return quads;
}

void find_objects(const Mat *frame, const vector<Mat> *prev_frames, Mat *movement)
{
	uint w = frame->cols;
	uint h = frame->rows;
	uint *mask;
	uint *quads;
	uint sqrw, sqrh;

	mask = create_mask(frame, prev_frames, 25.0);
	nearest_pixels(mask, w, h);
	quads = mask_to_quads(mask, w, h, &sqrw, &sqrh);	

	for (int y = 0; y < sqrh; y++) {
		for (int x = 0; x < sqrw; x++) { 
			if (quads[y * sqrw + x]) {
				Point pt1(QUAD_EDGE * x, QUAD_EDGE * y);
				Point pt2(QUAD_EDGE * x + QUAD_EDGE, QUAD_EDGE * y + QUAD_EDGE);

				rectangle(*movement, pt1, pt2,  Scalar(0, 255, 255),  2, 8, 0);
			}
		}
	}


	free(mask);
	free(quads);
}

int main(int argc, char** argv)
{
	Mat frame, clustered, movement;
	vector<Mat> prev_frames(1);

	// Init video capture device
	int devid = 0;

	if (argc > 1)
		devid = atoi(argv[1]);

	VideoCapture capt(devid);

	if (!capt.isOpened()) {
		cout << "Error opening default cam!" << endl;
		system("pause");
		return -1;
	}
	
	// Create windows
	namedWindow("Original", CV_WINDOW_AUTOSIZE);
	namedWindow("Clustered", CV_WINDOW_AUTOSIZE);
	namedWindow("Movement", CV_WINDOW_AUTOSIZE);

	capt.read(frame);
	prev_frames[0] = frame.clone();

	while (true) {	
		bool res = capt.read(frame);
		
		clustered = frame.clone();
		movement = frame.clone();

		if (!res) {
			cout << "Error reading frame!" << endl;
			system("pause");
			return -1;
		}

//		clust(&frame, &clustered, 2);

		find_objects(&clustered, &prev_frames, &movement);

		imshow("Original", frame);
//		imshow("Clustered", clustered);
		imshow("Movement", movement);
	
		// Key pressing events proccessing
		// If 'esc' key is pressed, break loop
		int key = waitKey(30);

		if (key == KEY_ESC) {
			cout << "esc key is pressed by user" << endl;
			break;
		} else {
			switch (key) {
			case KEY_ARROW_LEFT:
				break;
			case KEY_ARROW_UP:
				break;
			case KEY_ARROW_RIGHT:
				break;
			case KEY_ARROW_DOWN:
				break;
			case KEY_SPACE:
				break;
			}
		}

//		prev_frames.clear();
		prev_frames[0] = frame.clone();
	}

	return 0;
}
