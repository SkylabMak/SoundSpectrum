#ifndef WASAPI_AUDIO_H
#define WASAPI_AUDIO_H

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <Audiopolicy.h>
#include <vector>
#include <iostream>

class WasapiAudio
{
public:
    WasapiAudio();
    ~WasapiAudio();

    bool initialize();                             // setup WASAPI
    bool startCapture();                           // เริ่ม Capture
    bool getAudioData(std::vector<float> &buffer); // ดึงข้อมูลเสียง
    void stopCapture();                            // หยุด Capture

    int getSamplingRate() const
    {
        return pwfx ? pwfx->nSamplesPerSec : 44100; // Default 44100 Hz ถ้า `pwfx` ไม่ถูกกำหนด
    }

private:
    WAVEFORMATEX *pwfx = nullptr;
    IMMDevice *pDevice = nullptr;
    IAudioClient *pAudioClient = nullptr;
    IAudioCaptureClient *pCaptureClient = nullptr;
};

#endif
