#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"

#define SAMPLE_RATE (44100)

typedef float SAMPLE;

PaStream *stream;

static int callback(const void *inputBuffer,
                    void *outputBuffer,
                    unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* timeInfo,
                    PaStreamCallbackFlags statusFlags,
                    void *userData)
{
    SAMPLE *out = (SAMPLE*)outputBuffer;
    const SAMPLE *in = (const SAMPLE*)inputBuffer;
    int i;
    for (i=0; i<framesPerBuffer; i++) {
        SAMPLE sample = *in++;
        *out++ = sample;
        *out++ = sample;
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

    int inputDeviceIndex = atoi(argv[1]);
    const PaDeviceInfo *inputInfo = Pa_GetDeviceInfo(inputDeviceIndex);
    if (inputInfo == NULL) {
        printf("INVALID INPUT DEVICE\n");
        return err;
    }

    int outputDeviceIndex = atoi(argv[2]);
    const PaDeviceInfo *outputInfo = Pa_GetDeviceInfo(outputDeviceIndex);
    if (outputInfo == NULL) {
        printf("INVALID OUTPUT DEVICE\n");
        return err;
    }

    printf("%s -> %s", inputInfo->name, outputInfo->name);

    inputParameters.device = inputDeviceIndex;
    inputParameters.channelCount = 1;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = 0.01;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    outputParameters.device = outputDeviceIndex;
    outputParameters.channelCount = 2;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = 0.01;
    outputParameters.hostApiSpecificStreamInfo = NULL;

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

    getchar();

    Pa_CloseStream(stream);
    Pa_Terminate();
}