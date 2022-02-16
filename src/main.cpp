// Dependencies
#include "emitter.hpp"
#include "capturer.hpp"
#include <cstdlib>
#include <math.h>
#include <time.h>
#include <iostream>
#include <string>

// Definitions
#define DEFAULT_SERVER_ADDRESS "192.168.0.100"
#define DEFAULT_SERVER_PORT 555
#define DEFAULT_PACKET_RATE 50

// Declarations
void ProcessData(char *data, int numFrames, int blockAlign, int channels);
void OnExit();

// Global variables
Emitter *emitter;
Capturer *capturer;
bool verbose;

// Main
int main(int argc, char *argv[])
{
    emitter = &Emitter(argc > 1 ? argv[1] : DEFAULT_SERVER_ADDRESS, argc > 2 ? std::stoi(argv[2]) : DEFAULT_SERVER_PORT);
    capturer = &Capturer(argc > 3 ? std::stoi(argv[3]) : DEFAULT_PACKET_RATE, &ProcessData);
    verbose = argc > 4 ? strcmp(argv[4], "true") == 0 || strcmp(argv[4], "t") == 0 : false;

    if (verbose)
    {
        std::cout
            << "Server address:    " << emitter->GetAddress() << std::endl
            << "Server port:       " << emitter->GetPort() << std::endl
            << "Packet rate:       " << capturer->GetFramesPerSecond() << std::endl;
    }

    std::atexit(&OnExit);

    verbose && (std::cout << "Capturing started..." << std::endl);

    emitter->Begin();
    capturer->Begin();

    verbose && (std::cout << "Capturing ended..." << std::endl);

    return 0;
}

double lastAverage = 0;
double lastLastAverage = 0;
int baseColor[3] = {0x7f, 0x25, 0x50};
int boostColor[3] = {0xff, 0x00, 0x00};

clock_t lastClock = clock();
int clockAverage = 0;

static void ProcessData(char *data, int numFrames, int blockAlign, int channels)
{
    if (verbose)
    {
        if (clockAverage >= 100)
        {
            clock_t currentClock = clock();
            std::cout << "Average packet rate: " << 1 / ((double)(currentClock - lastClock) / CLOCKS_PER_SEC / clockAverage) / 3 << std::endl;
            lastClock = currentClock;
            clockAverage = 0;
        }
        clockAverage++;
    }

    double averageSample = 0;
    int iterations = 0;

    for (int i = 0; i < numFrames * blockAlign; i += blockAlign)
    {
        float sample;
        unsigned char sampleBytes[] = {(unsigned char)data[i], (unsigned char)data[i + 1], (unsigned char)data[i + 2], (unsigned char)data[i + 3]};
        memcpy(&sample, &sampleBytes, sizeof(sample));
        if (sample < 0)
        {
            sample *= -1;
        }
        averageSample += sample;
        iterations++;
    }

    if (iterations == 0)
    {
        return;
    }

    averageSample /= iterations;
    averageSample = pow(averageSample, 0.66);
    averageSample = averageSample + (lastAverage - averageSample) * 0.5;
    averageSample = averageSample + (lastLastAverage - averageSample) * 0.25;
    lastLastAverage = lastAverage;
    lastAverage = averageSample;

    emitter->Emit((char)(baseColor[0] + (boostColor[0] - baseColor[0]) * averageSample),
                  (char)(baseColor[1] + (boostColor[1] - baseColor[1]) * averageSample),
                  (char)(baseColor[2] + (boostColor[2] - baseColor[2]) * averageSample));
}

void OnExit()
{
    verbose && (std::cout << "Cleaning up..." << std::endl);

    emitter->End();
    capturer->End();
}