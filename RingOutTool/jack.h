/*
This file is part of JackAutoEq.
Copyright 2013, Jamie Jessup

JackAutoEq is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Ardour Scene Manager is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Ardour Scene Manager. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef JACK_H
#define JACK_H
#define BASE_FREQ 440
#define UNUSED(x) (void)(x)

#include <iostream>
#include <string.h>
#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <fftw3.h>
#include <QVector>
#include <pthread.h>
#include "hannwindow.h"
#include "biquad_filter.h"

extern pthread_mutex_t spectrumDisplayMutex;

#define MAX_FILTERS 12

class MainWindow;

class Jack
{
    /*
      reference to main window so graph data can be changed
    */

    MainWindow *mainWindow;

    HannWindow hannWindow;

    /*
      Performs work on per sample basis. Currently does nothing.
      In the future adds samples to a buffer to take the FFT.
    */
    int process(jack_nframes_t nframes);

    /*
      Adjusts FFT parameters as nyDisplayCompensatedecessary if SR changes
    */
    int sampleRateCallback(jack_nframes_t nframes);

    jack_client_t*	client;
    jack_port_t*	inputPort;
    jack_port_t*	outputPort;

    /*
    Input and output fft buffers
    */
    double *fftAudioIn;
    fftw_complex *fftAudioOut;

    /*
    FFT plan data structure
    */
    fftw_plan inputPlan;

    int filterIndex;
    BiQuadFilter *filterBank;


    QVector<double> spectrumCompensation;
    QVector<double> averagedSpectrum;
    QVector<double> powerSpectrum;
    QVector<double> powerSpectrumdB;


    jack_ringbuffer_t *fftSampleBuffer;
    jack_ringbuffer_t *filterUpdateBuffer;

    void *fftThreadFunc(void *arg);
    static void *staticThread(void *arg);
    pthread_t fftThread;

    pthread_mutex_t fftThreadLock;
    pthread_cond_t  dataReady;

public:
    /*
        Constructs a Jack object, performing some client initialization
      */
    Jack(MainWindow *);
    ~Jack();

    /*
      Returns the number of frequency bins used by the FFT calculation for plotting
    */
    int getFrequencyBins(void);

    /*
      Returns the ring buffer pointer so the GUI can tell the DSP to add a filter
      */
    jack_ringbuffer_t *getFilterUpdateBuffer();

    //Current sample rate of the system
    int sample_rate;

    /*
      Number of bins in the fft
    */
    int N;

    /*
      Activates Jack Client
    */
    void activate();


    /*
    Called by the jack server when ever there is work to be done, internally calls the process function
    */
    static int staticProcess(jack_nframes_t nframes, void *arg);
    static int staticSampleRateCallback(jack_nframes_t nframes, void *arg);
};

#endif // JACK_H
