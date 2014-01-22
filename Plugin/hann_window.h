/*
This file is part of the Open Feedback Suppression LV2 Plugin
Copyright 2014, Jamie Jessup

Open Feedback Suppression is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Open Feedback Suppression is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Ardour Scene Manager. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HANN_WINDOW_H_
#define HANN_WINDOW_H_

#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846264338327

/**
 * A simple structure containing the length
 * and cefficients of a hann window
 */
typedef struct {
	int N;
	float *w;
} HannWindow;

/**
 * Generate a hann window on the heap
 *
 * And return a pointer to the generated window
 *
 */
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
