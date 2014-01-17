#include "hannwindow.h"


void HannWindow::init(int size)
{
    N = size;
    w = new float[N];

    for(int n=0; n<N; n++) {
        w[n] = 0.5*(1-cos((2*M_PI*n)/(N-1)));
    }

}
