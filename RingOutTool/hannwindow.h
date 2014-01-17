#ifndef HANNWINDOW_H
#define HANNWINDOW_H

#include <math.h>
#include <iostream>

class HannWindow
{
    int N;
public:
    float *w;
    void init(int size);
};

#endif // HANNWINDOW_H
