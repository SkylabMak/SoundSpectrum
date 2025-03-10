#include "WasapiAudio.h"
#include "config.h"
#define eLoopback (EDataFlow)1

WasapiAudio::WasapiAudio() {}

WasapiAudio::~WasapiAudio() {
    stopCapture();
}

bool WasapiAudio::initialize() {
    CoInitialize(NULL);
    IMMDeviceEnumerator* pEnumerator = nullptr; //ตัวแทน
    
    // หาอุปกรณ์เสียงที่กำลังใช้งาน
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) return false;

    // เลือกช่อง เพื่อดึงเสียงจากระบบ
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender,  eMultimedia, &pDevice);
    pEnumerator->Release();
    if (FAILED(hr)) return false;
    // เปิดใช้งาน WASAPI Audio Client
    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
    if (FAILED(hr)) return false;

    //  รับรูปแบบเสียงเพื่อที่จะกำหนด
    pAudioClient->GetMixFormat(&pwfx);

    std::cout << " Audio Sampling Rate: " << pwfx->nSamplesPerSec << " Hz" << std::endl;

    //  ตั้งค่า buffer ให้เหมาะกับ Real-time FFT
    REFERENCE_TIME bufferDuration = (REFERENCE_TIME)(((double)FFT_SAMPLES / pwfx->nSamplesPerSec) * 10000000.0);

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, bufferDuration, 0, pwfx, NULL);
    if (FAILED(hr)) return false;

    hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
    return SUCCEEDED(hr);
}

bool WasapiAudio::startCapture() {
    return SUCCEEDED(pAudioClient->Start());
}

bool WasapiAudio::getAudioData(std::vector<float>& buffer) {
    UINT32 packetLength = 0;
    pCaptureClient->GetNextPacketSize(&packetLength);
    
    if (packetLength > 0) {
        BYTE* pData; //raw audio data
        DWORD flags; // status
        UINT32 numFramesAvailable; //จำนวนเฟรมเสียงที่มีให้ดึง
        
        HRESULT hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
        if (FAILED(hr)) return false;

        //  ถ้า flags มีค่า SILENT แสดงว่าไม่มีเสียง ให้ return false
        if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
            buffer.assign(numFramesAvailable, 0.0f); //assign 0.0 to all e in numFramesAvailable size
        } else {
            float* floatData = reinterpret_cast<float*>(pData); // floatData and pData คือ จุดเริ่มต้นของข้อมูลเสียง
            buffer.assign(floatData, floatData + numFramesAvailable); //std::vector::assign(begin, end) copy with [begin, end)
        }

        pCaptureClient->ReleaseBuffer(numFramesAvailable); //คืนบัฟเฟอร์
        return true;
    }
    
    return false;
}

void WasapiAudio::stopCapture() {
    if (pAudioClient) pAudioClient->Stop();
    if (pCaptureClient) pCaptureClient->Release();
    if (pAudioClient) pAudioClient->Release();
    if (pDevice) pDevice->Release();
    CoUninitialize();
}
