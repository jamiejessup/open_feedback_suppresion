/*
This file is part of the Open Feedback Suppression Ring Out Tool
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

#include "hannwindow.h"


void HannWindow::init(int size)
{
    N = size;
    w = new float[N];

    for(int n=0; n<N; n++) {
        w[n] = 0.5*(1-cos((2*M_PI*n)/(N-1)));
    }

}
