/*
 * hann_window.h
 *
 *  Created on: Jan 19, 2014
 *      Author: jamie
 */

#ifndef HANN_WINDOW_H_
#define HANN_WINDOW_H_

#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846264338327

typedef struct {
	int N;
	float *w;
} HannWindow;

HannWindow *new_hann_window(int N) {
	HannWindow * hw = (HannWindow *) malloc(sizeof(HannWindow));
	hw->N = N;
	hw->w = (float*) malloc(N*sizeof(float));

	for(int n=0; n<N; n++) {
		hw->w[n] = 0.5*(1-cos((2*PI*n)/(N-1)));
	}

	return hw;
}

void free_hann_window(HannWindow *hw) {
	free(hw->w);
	free(hw);
}


#endif /* HANN_WINDOW_H_ */
