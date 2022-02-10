#include <winsock2.h>
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <time.h>
#include <iostream>

// Main code
#define SERVER_PORT 555
#define SERVER_ADDRESS "IP_ADDRESS"
#define SAMPLE_OPTIMIZATION 1

double lastAverage = 0;
double lastLastAverage = 0;
int serverSocket;
struct sockaddr_in serverAddr;
int baseColor[3] = { 0x7f, 0x25, 0x50 };
int boostColor[3] = { 0xff, 0x00, 0x00 };

void idontknowcpp(char *bytes, int numBytes)
{
    double averageSample = 0;
    int iterations = 0;
    for (int i = 0; i < numBytes; i += 8 * SAMPLE_OPTIMIZATION)
    {
        float sample;
        unsigned char sampleBytes[] = {(unsigned char)bytes[i], (unsigned char)bytes[i + 1], (unsigned char)bytes[i + 2], (unsigned char)bytes[i + 3]};
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

    char data[] = {
        0x10,
        (char)(baseColor[0] + (boostColor[0] - baseColor[0]) * averageSample),
        (char)(baseColor[1] + (boostColor[1] - baseColor[1]) * averageSample),
        (char)(baseColor[2] + (boostColor[2] - baseColor[2]) * averageSample)};
    sendto(serverSocket, data, 4, 0, (LPSOCKADDR)&serverAddr, sizeof(struct sockaddr));
}

// No idea what the rest does

using namespace std;

#pragma comment(lib, "Winmm.lib")

#define REFTIMES_PER_SEC 500000
#define REFTIMES_PER_MILLISEC 500

#define EXIT_ON_ERROR(hres) \
    if (FAILED(hres))       \
    {                       \
        goto Exit;          \
    }
#define SAFE_RELEASE(punk) \
    if ((punk) != NULL)    \
    {                      \
        (punk)->Release(); \
        (punk) = NULL;     \
    }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

class LoopbackSink
{
public:
    HRESULT CopyData(BYTE *pData, UINT32 NumFrames, WAVEFORMATEX *pwfx);
};

HRESULT WriteWaveHeader(HMMIO hFile, LPCWAVEFORMATEX pwfx, MMCKINFO *pckRIFF, MMCKINFO *pckData);
HRESULT FinishWaveFile(HMMIO hFile, MMCKINFO *pckRIFF, MMCKINFO *pckData);
HRESULT RecordAudioStream(LoopbackSink *pMySink);

int main()
{
    WSADATA wsaData;
    WSAStartup(0x0202, &wsaData);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    serverAddr.sin_port = htons((u_short)SERVER_PORT);

    serverSocket = socket(PF_INET, SOCK_DGRAM, 0);

    clock();

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    LoopbackSink AudioSink;
    RecordAudioStream(&AudioSink);

    CoUninitialize();

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

HRESULT LoopbackSink::CopyData(BYTE *pData, UINT32 NumFrames, WAVEFORMATEX *pwfx)
{
    HRESULT hr = S_OK;

    if (0 == NumFrames)
    {
        wprintf(L"IAudioCaptureClient::GetBuffer said to read 0 frames\n");
        return E_UNEXPECTED;
    }

#pragma prefast(suppress \
                : __WARNING_INCORRECT_ANNOTATION, "IAudioCaptureClient::GetBuffer SAL annotation implies a 1-byte buffer")

    idontknowcpp(reinterpret_cast<PCHAR>(pData), NumFrames * pwfx->nBlockAlign);

    return S_OK;
}

HRESULT RecordAudioStream(LoopbackSink *pMySink)
{
    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    REFERENCE_TIME hnsActualDuration;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    IAudioClient *pAudioClient = NULL;
    IAudioCaptureClient *pCaptureClient = NULL;
    WAVEFORMATEX *pwfx = NULL;
    UINT32 packetLength = 0;

    BYTE *pData;
    DWORD flags;

    MMCKINFO ckRIFF = {0};
    MMCKINFO ckData = {0};

    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void **)&pEnumerator);
    EXIT_ON_ERROR(hr)

    hr = pEnumerator->GetDefaultAudioEndpoint(
        eRender, eConsole, &pDevice);
    EXIT_ON_ERROR(hr)

    hr = pDevice->Activate(
        IID_IAudioClient, CLSCTX_ALL,
        NULL, (void **)&pAudioClient);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->GetMixFormat(&pwfx);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        hnsRequestedDuration,
        0,
        pwfx,
        NULL);
    EXIT_ON_ERROR(hr)

    // Get the size of the allocated buffer.
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->GetService(
        IID_IAudioCaptureClient,
        (void **)&pCaptureClient);
    EXIT_ON_ERROR(hr)

    // Calculate the actual duration of the allocated buffer.
    hnsActualDuration = (double)REFTIMES_PER_SEC *
                        bufferFrameCount / pwfx->nSamplesPerSec;

    hr = pAudioClient->Start(); // Start recording.
    EXIT_ON_ERROR(hr)

    // Each loop fills about half of the shared buffer.
    while (TRUE)
    {
        // Sleep for half the buffer duration.
        Sleep(hnsActualDuration / REFTIMES_PER_MILLISEC / 4);

        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        EXIT_ON_ERROR(hr)

        while (packetLength != 0)
        {
            // Get the available data in the shared buffer.
            hr = pCaptureClient->GetBuffer(
                &pData,
                &numFramesAvailable,
                &flags, NULL, NULL);
            EXIT_ON_ERROR(hr)

            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                pData = NULL; // Tell CopyData to write silence.
            }

            // Copy the available capture data to the audio sink.
            hr = pMySink->CopyData(
                pData, numFramesAvailable, pwfx);
            EXIT_ON_ERROR(hr)

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            EXIT_ON_ERROR(hr)

            hr = pCaptureClient->GetNextPacketSize(&packetLength);
            EXIT_ON_ERROR(hr)
        }
    }

    hr = pAudioClient->Stop(); // Stop recording.
    EXIT_ON_ERROR(hr)

Exit:
    CoTaskMemFree(pwfx);
    SAFE_RELEASE(pEnumerator)
    SAFE_RELEASE(pDevice)
    SAFE_RELEASE(pAudioClient)
    SAFE_RELEASE(pCaptureClient)

    return hr;
}