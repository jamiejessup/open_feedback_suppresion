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

#include "jack.h"
#include "mainwindow.h"


Jack::Jack(MainWindow *mw)
{

    mainWindow = mw;

    if ((client = jack_client_open("OFS Ring Out Tool", JackNullOption, NULL)) == 0)
    {
        std::cout << "jack server not running?" << std::endl;
    }

    //Get an input for measurement
    inputPort  = jack_port_register (client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    outputPort  = jack_port_register (client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    jack_set_process_callback (client, staticProcess, static_cast<void*>(this));

    jack_set_sample_rate_callback (client, staticSampleRateCallback, static_cast<void*>(this));

    //Initialize some FFT stuff. Set number of bins and the plan based on sample rate
    N = 24*2048;


    pthread_mutex_init(&fftThreadLock, NULL);
    pthread_cond_init (&dataReady, NULL);

    //Get sample rate
    sample_rate = (int) jack_get_sample_rate(client);
    fftAudioIn = (double*) fftw_malloc(sizeof(double) * N);
    fftAudioOut = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    inputPlan = fftw_plan_dft_r2c_1d(N, fftAudioIn, fftAudioOut, FFTW_ESTIMATE);


    //set size of buffers
    powerSpectrum.resize((N+1)/2);
    powerSpectrumdB.resize((N+1)/2);

    // create an instance of a ringbuffer
    fftSampleBuffer = jack_ringbuffer_create( N * sizeof(jack_default_audio_sample_t));

    // lock the buffer into memory, this is *NOT* realtime safe, do it before
    // using the buffer!
    int res = jack_ringbuffer_mlock(fftSampleBuffer);

    // check if we've locked the memory successfully
    if ( res ) {
        std::cout << "Error locking memory!" << std::endl;
    }

    // create an instance of a ringbuffer to rx filters to implement
    filterUpdateBuffer = jack_ringbuffer_create( 5 * sizeof(float));

    // lock the buffer into memory, this is *NOT* realtime safe, do it before
    // using the buffer!
    res = jack_ringbuffer_mlock(filterUpdateBuffer);

    // check if we've locked the memory successfully
    if ( res ) {
        std::cout << "Error locking memory!" << std::endl;
    }

    //now allocate a block of memory enough for the filter bank
    filterBank = (BiQuadFilter*) malloc(MAX_FILTERS *sizeof(BiQuadFilter));

    for(unsigned i = 0; i<MAX_FILTERS; i++){
        filterBank[i].enabled = false;
    }

    hannWindow.init(N);



    pthread_create(&fftThread, NULL, staticThread, static_cast<void*>(this));

}

Jack::~Jack()
{
    pthread_mutex_destroy(&fftThreadLock);
    pthread_cond_destroy(&dataReady);
    fftw_destroy_plan(inputPlan);
    fftw_free(fftAudioIn); fftw_free(fftAudioOut);
    free(filterBank);
}

void Jack::activate()
{

    if (jack_activate(client) != 0)
    {
        std::cout<<  "cannot activate client" << std::endl;
        return;
    }

    //Automatically connect to output ports 1 and 2, user can change others
    const char **ports;
    ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical);
    int i = 0;
    while (ports[i] != NULL) {
        if (strcmp(ports[i], "system:capture_1") == 0) {
            jack_connect(client, ports[i], jack_port_name(inputPort));
        }
        if (strcmp(ports[i], "system:playback_1") == 0) {
            jack_connect(client, jack_port_name(outputPort), ports[i]);
        }
        if (strcmp(ports[i], "system:playback_2") == 0) {
            jack_connect(client, jack_port_name(outputPort), ports[i]);
        }
        i++;
    }
}

int Jack::staticProcess(jack_nframes_t nframes, void *arg)
{
    return static_cast<Jack*>(arg)->process(nframes);
}

int Jack::process(jack_nframes_t nframes)
{

    //Get the audio sample from jack
    jack_default_audio_sample_t *audioIn =
            (jack_default_audio_sample_t *)
            jack_port_get_buffer (inputPort, nframes);

    //check to see if we need to do additional processing

    unsigned availableRead = jack_ringbuffer_read_space(filterUpdateBuffer);

    if ( availableRead >= sizeof(float) ) {
        float fc;
        jack_ringbuffer_read(filterUpdateBuffer,(char*)&fc,sizeof(float));

        if(filterIndex != MAX_FILTERS){
            addNotchFilterToBank(filterBank,fc,sample_rate,filterIndex,25);
            filterIndex++;
            std::cout << fc << std::endl;
        }
    }

    //process the output with notches and
    //send the processed output to the FFT thread.
    jack_default_audio_sample_t *audioOut = (jack_default_audio_sample_t*) jack_port_get_buffer (outputPort, nframes);
    for(unsigned int i = 0; i<nframes;i++){
        audioOut[i] = processFilterBank(filterBank,audioIn[i],MAX_FILTERS);
        jack_ringbuffer_write(fftSampleBuffer,(char*) &audioOut[i], sizeof(jack_default_audio_sample_t));
    }

    if (pthread_mutex_trylock (&fftThreadLock) == 0) {
        pthread_cond_signal (&dataReady);
        pthread_mutex_unlock (&fftThreadLock);
    }

    return 0;
}

int Jack::staticSampleRateCallback(jack_nframes_t nframes, void *arg)
{
    return static_cast<Jack*>(arg)->sampleRateCallback(nframes);
}

int Jack::sampleRateCallback (jack_nframes_t nframes)
{
    sample_rate = nframes;

    return 0;
}

int Jack::getFrequencyBins(void) {
    return N/2;
}

void * Jack::staticThread(void *arg) {
    return static_cast<Jack*>(arg)->fftThreadFunc(NULL);
}

void *Jack::fftThreadFunc(void *arg){

    UNUSED(arg);
    int sampleCount = 0;
    unsigned shift = 0;
    jack_default_audio_sample_t sample;

    pthread_mutex_lock(&fftThreadLock);
    while(true){
        pthread_cond_wait(&dataReady, &fftThreadLock);

        unsigned availableRead = jack_ringbuffer_read_space(fftSampleBuffer);

        if ( availableRead >= sizeof(jack_default_audio_sample_t) ) {
            shift = availableRead/sizeof(jack_default_audio_sample_t);
            for(unsigned i = 0; i<shift; i++){
                jack_ringbuffer_read(fftSampleBuffer,(char*)&sample,sizeof(jack_default_audio_sample_t));
                fftAudioIn[sampleCount] = sample;
                sampleCount++;

                if(sampleCount >= N-1){
                    sampleCount = 0;

                    //do some windowing


                    for(int i=0; i<N; i++){
                        fftAudioIn[i] = fftAudioIn[i]*hannWindow.w[i];
                    }


                    //Exectute the FFTs, results stored in FFT*Out buffers
                    fftw_execute(inputPlan);
                    //Prepare the results of the FFT to graph it


                    //normalize fft result

                    for (int i = 0; i < (N+1)/2; i++) {
                        fftAudioOut[i][0] /= N; fftAudioOut[i][1] /= N;
                    }

                    // (k < N/2 rounded up)
                    for (int i = 0; i < (N+1)/2; i++) {
                        powerSpectrum[i] = fftAudioOut[i][0]*fftAudioOut[i][0] + fftAudioOut[i][1]*fftAudioOut[i][1];
                        if(powerSpectrum[i] == 0)
                            powerSpectrumdB[i] = -120;
                        else
                            powerSpectrumdB[i] = 10*log10(powerSpectrum[i]);
                    }

                    // N is even
                    if (N % 2 == 0) {
                        powerSpectrum[N/2] = fftAudioOut[N/2][0]*fftAudioOut[N/2][0] + fftAudioOut[N/2][1]*fftAudioOut[N/2][1];
                        if(powerSpectrum[N/2] == 0)
                            powerSpectrumdB[N/2] = -120;
                        else
                            powerSpectrumdB[N/2] = 10*log10(powerSpectrum[N/2]);
                    }
                    mainWindow->yDisplayBuffer = powerSpectrumdB;

                }
            }
        }


    }

    pthread_mutex_unlock(&fftThreadLock);
    pthread_exit(NULL);
}

jack_ringbuffer_t *Jack::getFilterUpdateBuffer() {
    return filterUpdateBuffer;
}
