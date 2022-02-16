#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functional>
#include <stdio.h>

#pragma comment(lib, "Winmm.lib")

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

class Capturer
{
public:
    Capturer();
    Capturer(int framesPerSecond, void (*processEvent)(char *, int, int, int));
    void Begin();
    void End();

    int GetFramesPerSecond();

private:
    class Sink
    {
    public:
        void (*event)(char *, int, int, int);
        HRESULT CopyData(BYTE *pData, UINT32 numFrames, WAVEFORMATEX *pwfx);
    };

    int refTimes;
    HRESULT Capturer::RecordFrames(Capturer::Sink *pSink, int delay);
    Sink audioSink;

    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    const IID IID_IAudioClient = __uuidof(IAudioClient);
    const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
};