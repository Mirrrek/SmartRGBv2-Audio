#include "capturer.hpp"

// Constructor
Capturer::Capturer(int framesPerSecond, void (*processEvent)(char *, int, int, int))
{
    refTimes = framesPerSecond;
    audioSink = Sink();
    audioSink.event = processEvent;
}

// Initializes capturer
void Capturer::Begin()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    RecordFrames(&audioSink, 1000 / refTimes);
}

// Uninitializes capturer
void Capturer::End()
{
    CoUninitialize();
}

int Capturer::GetFramesPerSecond() {
    return refTimes;
}

// Captures frames
HRESULT Capturer::Sink::CopyData(BYTE *pData, UINT32 numFrames, WAVEFORMATEX *pwfx)
{
    HRESULT hr = S_OK;

    if (0 == numFrames)
    {
        return E_UNEXPECTED;
    }

    // #pragma prefast(suppress \
//                 : __WARNING_INCORRECT_ANNOTATION, "IAudioCaptureClient::GetBuffer SAL annotation implies a 1-byte buffer")

    event(reinterpret_cast<PCHAR>(pData), numFrames, pwfx->nBlockAlign, pwfx->nChannels);

    return S_OK;
}

// Records frames
HRESULT Capturer::RecordFrames(Capturer::Sink *pSink, int delay)
{
    HRESULT hr;
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
        Capturer::CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, Capturer::IID_IMMDeviceEnumerator,
        (void **)&pEnumerator);
    EXIT_ON_ERROR(hr)

    hr = pEnumerator->GetDefaultAudioEndpoint(
        eRender, eConsole, &pDevice);
    EXIT_ON_ERROR(hr)

    hr = pDevice->Activate(
        Capturer::IID_IAudioClient, CLSCTX_ALL,
        NULL, (void **)&pAudioClient);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->GetMixFormat(&pwfx);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        0,
        0,
        pwfx,
        NULL);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->GetService(
        Capturer::IID_IAudioCaptureClient,
        (void **)&pCaptureClient);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->Start();
    EXIT_ON_ERROR(hr)

    while (TRUE)
    {
        Sleep(delay);

        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        EXIT_ON_ERROR(hr)

        while (packetLength != 0)
        {
            hr = pCaptureClient->GetBuffer(
                &pData,
                &numFramesAvailable,
                &flags, NULL, NULL);
            EXIT_ON_ERROR(hr)

            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                pData = NULL;
            }

            hr = pSink->CopyData(
                pData, numFramesAvailable, pwfx);
            EXIT_ON_ERROR(hr)

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            EXIT_ON_ERROR(hr)

            hr = pCaptureClient->GetNextPacketSize(&packetLength);
            EXIT_ON_ERROR(hr)
        }
    }

    hr = pAudioClient->Stop();
    EXIT_ON_ERROR(hr)

Exit:
    CoTaskMemFree(pwfx);
    SAFE_RELEASE(pEnumerator)
    SAFE_RELEASE(pDevice)
    SAFE_RELEASE(pAudioClient)
    SAFE_RELEASE(pCaptureClient)

    return hr;
}