#include "VolumeSetter.h"

#include <windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>

bool VolumeSetter::SetAppVolume(const std::wstring& targetProcessName, float volumeLevel)
{
    if (volumeLevel < 0.0f) volumeLevel = 0.0f;
    if (volumeLevel > 1.0f) volumeLevel = 1.0f;

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
    {
        return false;
    }

    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioSessionManager2* pSessionManager = nullptr;
    IAudioSessionEnumerator* pSessionEnumerator = nullptr;
    bool success = false;
    int sessionCount = 0;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);

    if (FAILED(hr)) goto Cleanup;

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr)) goto Cleanup;

    hr = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL,
        NULL, (void**)&pSessionManager);
    if (FAILED(hr)) goto Cleanup;

    hr = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
    if (FAILED(hr)) goto Cleanup;


    hr = pSessionEnumerator->GetCount(&sessionCount);
    if (FAILED(hr)) goto Cleanup;

    for (int i = 0; i < sessionCount; ++i) 
    {
        IAudioSessionControl* pSessionControl = nullptr;
        IAudioSessionControl2* pSessionControl2 = nullptr;
        ISimpleAudioVolume* pSimpleAudioVolume = nullptr;

        hr = pSessionEnumerator->GetSession(i, &pSessionControl);
        if (FAILED(hr)) continue;

        // Query for the extended session control interface
        hr = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pSessionControl2);
        if (SUCCEEDED(hr)) 
        {
            LPWSTR sessionSessionIdentifier = nullptr;
            pSessionControl2->GetSessionIdentifier(&sessionSessionIdentifier);

            std::wstring sessionPath(sessionSessionIdentifier);
            CoTaskMemFree(sessionSessionIdentifier);

            if (sessionPath.find(targetProcessName) != std::wstring::npos) 
            {
                hr = pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&pSimpleAudioVolume);
                if (SUCCEEDED(hr)) 
                {
                    hr = pSimpleAudioVolume->SetMasterVolume(volumeLevel, NULL);
                    if (SUCCEEDED(hr)) 
                    {
                        success = true;
                    }
                    pSimpleAudioVolume->Release();
                }
            }
            pSessionControl2->Release();
        }
        pSessionControl->Release();

        if (success) break;
    }

Cleanup:
    if (pSessionEnumerator) pSessionEnumerator->Release();
    if (pSessionManager) pSessionManager->Release();
    if (pDevice) pDevice->Release();
    if (pEnumerator) pEnumerator->Release();
    CoUninitialize();
    
    return success;
}

float VolumeSetter::GetAppVolume(const std::wstring& targetProcessName)
{
	float volumeLevel = 0.0f;

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
    {
        return false;
    }

    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioSessionManager2* pSessionManager = nullptr;
    IAudioSessionEnumerator* pSessionEnumerator = nullptr;
    bool success = false;
    int sessionCount = 0;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);

    if (FAILED(hr)) goto Cleanup;

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr)) goto Cleanup;

    hr = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL,
        NULL, (void**)&pSessionManager);
    if (FAILED(hr)) goto Cleanup;

    hr = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
    if (FAILED(hr)) goto Cleanup;


    hr = pSessionEnumerator->GetCount(&sessionCount);
    if (FAILED(hr)) goto Cleanup;

    for (int i = 0; i < sessionCount; ++i)
    {
        IAudioSessionControl* pSessionControl = nullptr;
        IAudioSessionControl2* pSessionControl2 = nullptr;
        ISimpleAudioVolume* pSimpleAudioVolume = nullptr;

        hr = pSessionEnumerator->GetSession(i, &pSessionControl);
        if (FAILED(hr)) continue;

        // Query for the extended session control interface
        hr = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pSessionControl2);
        if (SUCCEEDED(hr))
        {
            LPWSTR sessionSessionIdentifier = nullptr;
            pSessionControl2->GetSessionIdentifier(&sessionSessionIdentifier);

            std::wstring sessionPath(sessionSessionIdentifier);
            CoTaskMemFree(sessionSessionIdentifier);

            if (sessionPath.find(targetProcessName) != std::wstring::npos)
            {
                hr = pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&pSimpleAudioVolume);
                if (SUCCEEDED(hr))
                {
                    //hr = pSimpleAudioVolume->SetMasterVolume(volumeLevel, NULL);
                    hr = pSimpleAudioVolume->GetMasterVolume(&volumeLevel);
                    if (SUCCEEDED(hr))
                    {
                        success = true;
                    }
                    pSimpleAudioVolume->Release();
                }
            }
            pSessionControl2->Release();
        }
        pSessionControl->Release();

        if (success) break;
    }
Cleanup:
    if (pSessionEnumerator) pSessionEnumerator->Release();
    if (pSessionManager) pSessionManager->Release();
    if (pDevice) pDevice->Release();
    if (pEnumerator) pEnumerator->Release();
    CoUninitialize();

    return volumeLevel;
}