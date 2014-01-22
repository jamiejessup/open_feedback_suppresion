#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PI 3.14159265358979323846264338327

typedef struct biQuadFilt{
    bool enabled;
    bool is_notch;
    float fc;
    float fs;
    float b0;
    float b1;
    float b2;
    float a0;
    float a1;
    float a2;
    float y[3];
    float x[3];
    float alpha;
    float q;
    float gain;
} BiQuadFilter;

float processFilter(BiQuadFilter *filter, float input);
float processFilterBank(BiQuadFilter *filterBank, float input, unsigned bankSize);
void addFilterToBank(BiQuadFilter *filterBank, float fc, float fs, int index,float gain, float q);
void addNotchFilterToBank(BiQuadFilter *filterBank, float fc, float fs, int index, float q);
void addNotchFilterToBank(BiQuadFilter *filterBank, float fc, float fs, int index, float q);
void newFilterGain(BiQuadFilter *filterBank, int index, float gain);

#ifdef __cplusplus
}
#endif
