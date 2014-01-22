/*
 * Author: Jamie Jessup 2013
 *         jessup.jamie@gmail.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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


void addFilterToBank(BiQuadFilter *filterBank, float fc, float fs, int index, float gain, float q) {
	if(filterBank != NULL){
		filterBank[index].enabled = true;
		filterBank[index].is_notch = false;
		filterBank[index].fc = fc;
		filterBank[index].fs = fs;


		float w0 = 2*PI*fc/fs;
		float wS = sin(w0);
		float wC = cos(w0);
		float A = sqrt(pow(10,gain/20));
		float alpha = wS/(2*q);


		//zeroes
		filterBank[index].b0 = 1 + alpha*A;
		filterBank[index].b1 = -2*wC;
		filterBank[index].b2 = 1 - alpha*A;
		//poles
		filterBank[index].a0 = 1+alpha/A;
		filterBank[index].a1 = -2*wC;
		filterBank[index].a2 = 1-alpha/A;

		//set i/o to 0

		filterBank[index].x[0] = 0;
		filterBank[index].x[1] = 0;
		filterBank[index].x[2] = 0;
		filterBank[index].y[0] = 0;
		filterBank[index].y[1] = 0;
		filterBank[index].y[2] = 0;
	}
}

void addNotchFilterToBank(BiQuadFilter *filterBank, float fc, float fs, int index, float q) {
	if(filterBank != NULL){
		filterBank[index].enabled = true;
		filterBank[index].is_notch = true;
		filterBank[index].fc = fc;
		filterBank[index].fs = fs;


		float w0 = 2*PI*fc/fs;
		float wS = sin(w0);
		float wC = cos(w0);
		float alpha = wS/(2*q);


		//zeroes
		filterBank[index].b0 = 1;
		filterBank[index].b1 = -2*wC;
		filterBank[index].b2 = 1;
		//poles
		filterBank[index].a0 = 1+alpha;
		filterBank[index].a1 = -2*wC;
		filterBank[index].a2 = 1-alpha;

		//set i/o to 0

		filterBank[index].x[0] = 0;
		filterBank[index].x[1] = 0;
		filterBank[index].x[2] = 0;
		filterBank[index].y[0] = 0;
		filterBank[index].y[1] = 0;
		filterBank[index].y[2] = 0;
	}
}

void newFilterGain(BiQuadFilter *filterBank, int index, float gain){
	if(!filterBank[index].is_notch){

		float w0 = 2*PI*filterBank[index].fc/filterBank[index].fs;
		float wS = sin(w0);
		float wC = cos(w0);
		float alpha = wS/(2*filterBank[index].q);
		float A = sqrt(pow(10,gain/20));

		//zeroes
		filterBank[index].b0 = 1 + alpha*A;
		filterBank[index].b1 = -2*wC;
		filterBank[index].b2 = 1 - alpha*A;
		//poles
		filterBank[index].a0 = 1+alpha/A;
		filterBank[index].a1 = -2*wC;
		filterBank[index].a2 = 1-alpha/A;
	}
}
