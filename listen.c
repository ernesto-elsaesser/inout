#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "portaudio.h"

#define SAMPLE_RATE (44100)
#define FILTER_FREQUENCY (2000)
#define FILTER_Q (0.707)
#define FILTER_GAIN (30)

#ifndef M_PI
#define M_PI (3.141592654)
#endif

PaStream *stream;

float b0, b1, b2, a1, a2;

float prev_input_1 = 0.0;
float prev_input_2 = 0.0;
float prev_output_1 = 0.0;
float prev_output_2 = 0.0;


static int callback(const void *inputBuffer,
                    void *outputBuffer,
                    unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* timeInfo,
                    PaStreamCallbackFlags statusFlags,
                    void *userData)
{
    float *out = (float*)outputBuffer;
    const float *in = (const float*)inputBuffer;
    int i;
    for (i=0; i<framesPerBuffer; i++) {
        float input = *in++;
        float output = (b0 * input) +
                (b1 * prev_input_1) +
                (b2 * prev_input_2) -
                (a1 * prev_output_1) -
                (a2 * prev_output_2);
        prev_input_2 = prev_input_1;
        prev_input_1 = input;
        prev_output_2 = prev_output_1;
        prev_output_1 = output;
        *out++ = output;
        *out++ = output;
    }
    return paContinue;
}

int main(int argc, char *argv[])
{
    PaStreamParameters inputParameters, outputParameters;

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        printf("Pa_Initialize: %s\n", Pa_GetErrorText(err));
        return err;
    }

    if (argc < 3) {
        PaDeviceIndex deviceCount = Pa_GetDeviceCount();
        PaDeviceIndex deviceIndex;
        const PaDeviceInfo *deviceInfo;
        for (deviceIndex=0; deviceIndex<deviceCount; deviceIndex++) {
            deviceInfo = Pa_GetDeviceInfo(deviceIndex);
            printf("%i: %s (%i in, %i out)\n", deviceIndex, deviceInfo->name,
                deviceInfo->maxInputChannels, deviceInfo->maxOutputChannels);
        }
        return 0;
    }

    inputParameters.device = atoi(argv[1]);
    inputParameters.channelCount = 1;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = 0.01;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    outputParameters.device = atoi(argv[2]);
    outputParameters.channelCount = 2;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = 0.01;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    float omega = 2 * M_PI * FILTER_FREQUENCY / SAMPLE_RATE;
    float sn = sin(omega);
    float cs = cos(omega);
    float alpha = sn / (2 * FILTER_Q);

    float A = pow(10, FILTER_GAIN / 40); // convert to db
    float beta = sqrt(A + A);

    // lowshelf
    b0 = A * ((A + 1) - (A - 1) * cs + beta * sn);
    b1 = 2 * A * ((A - 1) - (A + 1) * cs);
    b2 = A * ((A + 1) - (A - 1) * cs - beta * sn);
    float a0 = (A + 1) + (A - 1) * cs + beta * sn;
    a1 = -2 * ((A - 1) + (A + 1) * cs);
    a2 = (A + 1) + (A - 1) * cs - beta * sn;

    // scale
    a1 /= (a0);
    a2 /= (a0);
    b0 /= (a0);
    b1 /= (a0);
    b2 /= (a0);

    err = Pa_OpenStream(&stream,
                        &inputParameters,
                        &outputParameters,
                        SAMPLE_RATE,
                        paFramesPerBufferUnspecified,
                        paNoFlag,
                        callback,
                        NULL);

    if (err != paNoError) {
        printf("Pa_OpenStream: %s\n", Pa_GetErrorText(err));
        return err;
    }
    
    err = Pa_StartStream(stream);

    if (err != paNoError) {
        printf("Pa_StartStream: %s\n", Pa_GetErrorText(err));
        return err;
    }

    getchar();

    Pa_CloseStream(stream);
    Pa_Terminate();
}