#include "opencv2/opencv.hpp"

uchar metric(uchar a, uchar b)
{
	return abs(a - b);
}

void assign_centroids(uchar *sample_data, uint *sample_centroids, uint len,
	uchar *means, uint k)
{
	int i, j;

	for (i = 0; i < len; ++i) {
		uchar min_dist = metric(sample_data[i], means[0]);
		int min_id = 0;

		for (j = 1; j < k; ++j) {
			uchar dist = metric(sample_data[i], means[j]);
		
			if (dist < min_dist) {
				min_dist = dist;
				min_id = j;
			}

		}

		sample_centroids[i] = min_id;
	}
}

int recompute_centroids(uchar *sample_data, uint *sample_centroids, uint len,
	uchar *means, uint k)
{
	uint *s = (uint *) malloc(sizeof(uint) * k);
	uint *n = (uint *) malloc(sizeof(uint) * k);
	int i;
	int stop = 1;

	for (i = 0; i < k; ++i) {
		s[i] = 0.0;
		n[i] = 0;
	}

	for (i = 0; i < len; ++i) {
		s[sample_centroids[i]] += sample_data[i];
		++n[sample_centroids[i]];
	}

	for (i = 0; i < k; ++i) {
		if ( !n[i] )
			  continue;

		s[i] /= n[i];	

		if (s[i] - means[i] == 0)
			stop = 0;

		means[i] = s[i];
	}

	free(s);
	free(n);
	s = NULL;
	n = NULL;

	return stop;
}

void init_centroids(uchar *sample_data, uint len, uchar *means, uint k)
{
	uint d = 255 / (k - 1);
	uint i;

	means[0] = 0;
	for (i = 1; i < k; i++)
		means[i] = means[i-1] + d;
}

uint *k_means(uchar *sample_data, uint len, uint k, uchar **means)
{
	*means = (uchar *) malloc(sizeof(uchar) * k);
	uint *sample_centroids = (uint *) calloc(len, sizeof(uint));

	init_centroids(sample_data, len, *means, k);

	int i = 0;
	do {
		i++;
		assign_centroids(sample_data, sample_centroids, len,
			*means, k);
	} while (!recompute_centroids(sample_data, sample_centroids, len,
		*means, k) && i < 5);

	return sample_centroids;
}
