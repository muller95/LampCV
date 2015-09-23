#include <cv.hpp>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

using namespace cv;
using namespace std;

#define QUAD_EDGE 25

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
	const Mat *prev_frame, double *hsv, double *prev_hsv, float coldist)
{
	uint w = frame->cols;
	uint h = frame->rows;

	// mask indices start from 1, all elements, that have 0 and h + 1 as
	// one of their indices, are filled with 0
	uint *mask = (uint *) calloc((w + 2) * (h + 2), sizeof(uint));


	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			color c0 (frame->data[frame->channels() * (w * y + x) + 0],
				frame->data[frame->channels() * (w * y + x) + 1],
				frame->data[frame->channels() * (w * y + x) + 2]);
			color c1 (prev_frame->data[prev_frame->channels() * (w * y + x) + 0],
				prev_frame->data[prev_frame->channels() * (w * y + x) + 1],
				prev_frame->data[prev_frame->channels() * (w * y + x) + 2]);

			double h = hsv[3 * (w * y + x) + 0],
				s = hsv[3 * (w * y + x) + 1];

			double prh = prev_hsv[3 * (w * y + x) + 0],
				prs = prev_hsv[3 * (w * y + x) + 1];

			if (color_dist(&c0, &c1) > coldist && (fabs(h - prh) >= 30.0 || fabs(s - prs) >= 0.1))
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

double *to_hsv(const Mat *frame)
{
	uint w = frame->cols;
	uint h = frame->rows;

	double *hsv = (double *)malloc(sizeof(double) * w * h * 3);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int mtype;
			double r, g, b;
			double max, min, h, s, v;
				
			r = (double)frame->data[frame->channels() * (w * y + x) + 0] / 255.0;
			g = (double)frame->data[frame->channels() * (w * y + x) + 1] / 255.0;
			b = (double)frame->data[frame->channels() * (w * y + x) + 2] / 255.0;
			
			max = min = r;
			mtype = 0;

			if (g > max) {
				max = g;
				mtype = 1;
			}

			if (b > max) {
				max = b;
				mtype = 2;	
			}

			min = (g < min) ? g : min;
			min = (b < min) ? b : min;

			v = max;
			s = (max == 0) ? 0 : 1.0 - min / max;
			
			if (max == min) {
				h = 0;
			} else if (mtype == 0) {
				h = 60 * (g - b) / (max - min) + (g >= b)? 0.0 : 360.0;
			} else if (mtype == 1) {
				h = 60 * (b - r) / (max - min) + 120.0;
			} else if (mtype == 2) {
				h = 60 * (r - g) / (max - min) + 240.0;
			}
		
			hsv[3 * (w * y + x) + 0] = h;
			hsv[3 * (w * y + x) + 1] = s;
			hsv[3 * (w * y + x) + 2] = v;
		}
	}

	return hsv;
}

int sort_func(const void *a, const void *b)
{
	return *((const uchar*) a) - *((const uchar *) b);
}

void fill_window(const Mat *frame, uchar *tmp_arr, uint frame_w,
	uint x, uint y, uint channel, uint win_size)
{
	uint o = 0;	
	
	for (int i = -((int)win_size/2); i <= ((int)win_size/2); i++)
		for (int j = -((int)win_size/2); j <= ((int)win_size/2); j++)
			tmp_arr[o++] = frame->data[frame->channels()
				* (frame_w * (y + i) + (x + j)) + channel];
}

void median_filter(const Mat *frame, int win_size)
{
	Mat tmp = frame->clone();
	uint w = frame->cols;
	uint h = frame->rows;
	uchar tmp_arr[win_size * win_size];

	int sidesz = (win_size - 1) / 2;
	for (int c = 0; c < frame->channels(); c++)
		for (int y = sidesz; y < h-sidesz; y++)
			for (int x = sidesz; x < w-sidesz; x++) {
				
				fill_window(frame, tmp_arr, w, x, y, c, win_size);

				qsort(tmp_arr, win_size * win_size, sizeof(uchar), sort_func);

				int wsz_sqr = win_size * win_size;
				frame->data[frame->channels() * (w * y + x) + c] = tmp_arr[wsz_sqr/2];
			}
		
}

double gauss_curve(double x, double sigma)
{
	return (exp(-x * x / 2.0 / sigma / sigma));
}

void gauss_blur_point_x(const uchar *frame_data, double *tmp_arr, int y, int x,
	int w, int h, int channels, double *weights, int sigma3)
{
	double r = 0.0, g = 0.0, b = 0.0;
	double s = 0.0;	

	for (int i = -sigma3; i <= sigma3; ++i)
	{
		int cur_x = x + i;
		cur_x = (cur_x < 0) ? 0 : cur_x;
		cur_x = (cur_x > (w - 1)) ? (w - 1) : cur_x;
		
		r += weights[i + sigma3] * (double)(frame_data[channels * (w * y + cur_x) + 0]);
		g += weights[i + sigma3] * (double)(frame_data[channels* (w * y + cur_x) + 1]);
		b += weights[i + sigma3] * (double)(frame_data[channels * (w * y + cur_x) + 2]);
		
		s += weights[i + sigma3];
	}

	tmp_arr[channels * (w * y + x) + 0] = r / s;
	tmp_arr[channels * (w * y + x) + 1] = g / s;
	tmp_arr[channels * (w * y + x) + 2] = b / s;
}

void gauss_blur_point_y(uchar *frame_data, const double *tmp_arr, int y, int x,
	int w, int h, int channels, double *weights, int sigma3)
{
	double r = 0.0, g = 0.0, b = 0.0;
	double s = 0.0;	

	for (int i = -sigma3; i <= sigma3; ++i)
	{
		int cur_y = y + i;
		cur_y = (cur_y < 0) ? 0 : cur_y;
		cur_y = (cur_y > (h - 1)) ? (h - 1) : cur_y;

		r += weights[i + sigma3] * (double)(frame_data[channels * (w * cur_y + x) + 0]);
		g += weights[i + sigma3] * (double)(frame_data[channels * (w * cur_y + x) + 1]);
		b += weights[i + sigma3] * (double)(frame_data[channels * (w * cur_y + x) + 2]);

		s += weights[i + sigma3];
	}

	frame_data[channels * (w * y + x) + 0] = r / s;
	frame_data[channels * (w * y + x) + 1] = g / s;
	frame_data[channels * (w * y + x) + 2] = b / s;
}

void gauss_blur(const Mat *frame, double sigma)
{
	uint w = frame->cols;
	uint h = frame->rows;
	double *tmp_arr = (double *)calloc(w * h * frame->channels(), sizeof(double));
	int sigma3 = (int)(3.0 * sigma);
	double *weights = (double *)malloc(sizeof(double) * (2 * sigma3 + 1));
	
	for (int i = 0; i <= 2*sigma3; ++i)
		weights[i] = gauss_curve(i - sigma3, sigma);

	for (int y = 0; y < h; ++y)
		for (int x = 0; x < w; ++x)
			gauss_blur_point_x(frame->data, tmp_arr, y, x, w, h,
				frame->channels(), weights, sigma3);

	for (int y = 0; y < h; ++y)
		for (int x = 0; x < w; ++x)
			gauss_blur_point_y(frame->data, tmp_arr, y, x, w, h,
				frame->channels(), weights, sigma3);

	free(weights);
	free(tmp_arr);
}

void window_blur(const Mat *frame) 
{
	uint w = frame->cols;
	uint h = frame->rows;
	int channels = frame->channels();

	uchar *frame_data = frame->data;

	for (int y = 1; y <= h - 1; y++) {
		for (int x = 1; x <= w - 1; x++) {
			for (int i = 0; i < frame->channels(); i++) {
				uint sum = 0.0;
				uint neightbours = 0;

				sum += frame_data[channels * (w * y - 1 + x - 1) + i];
				sum += frame_data[channels * (w * y - 1 + x) + i];
				sum += frame_data[channels * (w * y - 1 + x + 1) + i];
				
				sum += frame_data[channels * (w * y + x - 1) + i];
				sum += frame_data[channels * (w * y + x) + i];
				sum += frame_data[channels * (w * y + x + 1) + i];

				sum += frame_data[channels * (w * y + 1 + x - 1) + i];
				sum += frame_data[channels * (w * y + 1 + x) + i];
				sum += frame_data[channels * (w * y + 1 + x + 1) + i];

				sum /= 9;

				frame_data[channels * (w * y + x) + i] = (uchar)sum;
			}
		}
	}
}

void find_objects(const Mat *frame, Mat *prev_frame, Mat *movement)
{
	uint w = frame->cols;
	uint h = frame->rows;
	uint *mask;
	uint *quads;
	uint sqrw, sqrh;
	double *hsv;
	double *prev_hsv;


	*movement = frame->clone();

//	window_blur(frame);
//	window_blur(prev_frame);
	median_filter(frame, 3);
	median_filter(prev_frame, 3);
//	gauss_blur(frame, 1.0);
//	gauss_blur(prev_frame, 1.0);

	hsv = to_hsv(frame);
	prev_hsv = to_hsv(prev_frame);
	mask = create_mask(frame, prev_frame, hsv, prev_hsv, 25.0);
	nearest_pixels(mask, w, h);
	quads = mask_to_quads(mask, w, h, &sqrw, &sqrh);	



/*
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) { 		
			movement->data[frame->channels() * (w * y + x) + 0] = (uchar)(hsv[3 * (w * y + x) + 0] / 360.0 * 255.0);
			movement->data[frame->channels() * (w * y + x) + 1] = (uchar)(hsv[3 * (w * y + x) + 1] * 255.0);
			movement->data[frame->channels() * (w * y + x) + 2] = (uchar)(hsv[3 * (w * y + x) + 2] * 255.0);
			
		}
	}
*/

	for (int y = 0; y < sqrh; y++) {
		for (int x = 0; x < sqrw; x++) { 
			if (quads[y * sqrw + x]) {
				Point pt1(QUAD_EDGE * x, QUAD_EDGE * y);
				Point pt2(QUAD_EDGE * x + QUAD_EDGE, QUAD_EDGE * y + QUAD_EDGE);

				rectangle(*movement, pt1, pt2,  Scalar(0, 255, 255),  2, 8, 0);
			}
		}
	}


	free(prev_hsv);
	free(hsv);
	free(mask);
	free(quads);
}

int main(int argc, char** argv)
{
	Mat frame, clustered, movement;
	Mat prev_frame;
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

	capt.set(CV_CAP_PROP_FRAME_WIDTH,320);
	capt.set(CV_CAP_PROP_FRAME_HEIGHT,240);


	
	// Create windows
	namedWindow("Original", WINDOW_NORMAL);
	namedWindow("Movement", WINDOW_NORMAL);

	capt.read(frame);
	prev_frame = frame.clone();

	while (true) {	
		bool res = capt.read(frame);
		
		clustered = frame.clone();
		movement = frame.clone();

		if (!res) {
			cout << "Error reading frame!" << endl;
			system("pause");
			return -1;
		}

		find_objects(&clustered, &prev_frame, &movement);

		imshow("Original", frame);
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

		prev_frame = frame.clone();
	}

	return 0;
}
