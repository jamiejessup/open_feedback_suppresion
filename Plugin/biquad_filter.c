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

#include "biquad_filter.h"


float processFilter(BiQuadFilter *filter, float input) {
	//delay previous outputs/inputs
	filter->y[2] = filter->y[1];
	filter->y[1] = filter->y[0];
	filter->x[2] = filter->x[1];
	filter->x[1] = filter->x[0];
	filter->x[0] = input;

	//calculate the output
	filter->y[0] = (filter->b0/filter->a0)*filter->x[0] +
			(filter->b1/filter->a0)*filter->x[1] +
			(filter->b2/filter->a0)*filter->x[2] -
			(filter->a1/filter->a0)*filter->y[1] -
			(filter->a2/filter->a0)*filter->y[2];

	return filter->y[0];

}

float processFilterBank(BiQuadFilter *filterBank, float input, unsigned bankSize){
	float output = 0;
	//go through the filters in the list and process the ones enabled

	unsigned i=0;
	for(i=0; i<bankSize; i++){
		if(filterBank[i].enabled){
			output = processFilter(&(filterBank[i]),input);
			input = output;
		} else {
			output = input;
		}
	}

	return output;
}


void newPeakingFilter(BiQuadFilter *filter, float fc, float fs, float gain, float q) {
	if(filter != NULL){
		filter->enabled = true;
		filter->fc = fc;
		filter->fs = fs;
		filter->q = q;
		filter->gain = gain;


		float w0 = 2*PI*fc/fs;
		float wS = sin(w0);
		float wC = cos(w0);
		float A = sqrt(pow(10,gain/20));
		float alpha = wS/(2*q);


		//zeroes
		filter->b0 = 1 + alpha*A;
		filter->b1 = -2*wC;
		filter->b2 = 1 - alpha*A;
		//poles
		filter->a0 = 1+alpha/A;
		filter->a1 = -2*wC;
		filter->a2 = 1-alpha/A;

		//set i/o to 0

		filter->x[0] = 0;
		filter->x[1] = 0;
		filter->x[2] = 0;
		filter->y[0] = 0;
		filter->y[1] = 0;
		filter->y[2] = 0;
	}
}

void newNotchFilter(BiQuadFilter *filter, float fc, float fs, float q) {
	if(filter != NULL){
		filter->enabled = true;
		filter->fc = fc;
		filter->fs = fs;
		filter->q = q;
		filter->gain = 0;


		float w0 = 2*PI*fc/fs;
		float wS = sin(w0);
		float wC = cos(w0);
		float alpha = wS/(2*q);


		//zeroes
		filter->b0 = 1;
		filter->b1 = -2*wC;
		filter->b2 = 1;
		//poles
		filter->a0 = 1+alpha;
		filter->a1 = -2*wC;
		filter->a2 = 1-alpha;

		//set i/o to 0

		filter->x[0] = 0;
		filter->x[1] = 0;
		filter->x[2] = 0;
		filter->y[0] = 0;
		filter->y[1] = 0;
		filter->y[2] = 0;
	}
}

void newPeakingFilterCoeffs(BiQuadFilter *filter, float fc, float fs, float gain, float q) {
  if(fc > 20 && fc < 20000){
  filter->enabled = true;
		filter->fc = fc;
		filter->fs = fs;
		filter->gain = gain;
		filter->q = q;

		float w0 = 2*PI*fc/fs;
		float wS = sin(w0);
		float wC = cos(w0);
		float A = sqrt(pow(10,gain/20));
		float alpha = wS/(2*q);


		//zeroes
		filter->b0 = 1 + alpha*A;
		filter->b1 = -2*wC;
		filter->b2 = 1 - alpha*A;
		//poles
		filter->a0 = 1+alpha/A;
		filter->a1 = -2*wC;
		filter->a2 = 1-alpha/A;
		}
}

void newNotchFilterCoeffs(BiQuadFilter *filter, float fc, float fs, float q) {
  if(fc > 20 && fc < 20000){
		filter->enabled = true;
		filter->fc = fc;
		filter->fs = fs;
		filter->q = q;


		float w0 = 2*PI*fc/fs;
		float wS = sin(w0);
		float wC = cos(w0);
		float alpha = wS/(2*q);


		//zeroes
		filter->b0 = 1;
		filter->b1 = -2*wC;
		filter->b2 = 1;
		//poles
		filter->a0 = 1+alpha;
		filter->a1 = -2*wC;
		filter->a2 = 1-alpha;
		}
}


