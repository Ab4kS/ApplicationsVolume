#include <windows.h>
#include <mmdeviceapi.h>
#include <iostream>
#include <audioclient.h>
#include <audiopolicy.h>
#include <psapi.h>

#define EXIT_ON_ERROR(hres, err) if (FAILED(hres)) { std::cout<<err<<std::endl; return -4; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

int main() {
    HRESULT hr;
    hr = CoInitialize(NULL);
    EXIT_ON_ERROR(hr, "CoInitialize error")
    
    IMMDeviceEnumerator* pEnumerator = NULL;


    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (LPVOID*)&pEnumerator);
    EXIT_ON_ERROR(hr, "CoCreateInstance error");

    IMMDevice* defaultDevice = NULL;
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    EXIT_ON_ERROR(hr, "GetDefaultAudioEndpoint error");
    pEnumerator->Release();

    IAudioSessionManager2* sessionManager = NULL;
    hr = defaultDevice->Activate(__uuidof(sessionManager),
        CLSCTX_ALL, NULL, (LPVOID*)&sessionManager);
    EXIT_ON_ERROR(hr, "Activate error");
    defaultDevice->Release();

    IAudioSessionEnumerator* sessionEnum;
    hr = sessionManager->GetSessionEnumerator(&sessionEnum);
    EXIT_ON_ERROR(hr, "GetSessionEnumerator error");
    sessionManager->Release();

    int sessionCount;
    hr = sessionEnum->GetCount(&sessionCount);
    EXIT_ON_ERROR(hr, "GetCount error");
    std::cout << "Session count: " << sessionCount << std::endl;

    for (int session_id = 0; session_id < sessionCount; session_id++) 
    {
        IAudioSessionControl* sessionControl;
        hr = sessionEnum->GetSession(session_id, &sessionControl);
        EXIT_ON_ERROR(hr, "GetSession error");

        ISimpleAudioVolume* pSimpleAudioVolume;

        hr=sessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&pSimpleAudioVolume);
        EXIT_ON_ERROR(hr, "GetGroupingParam error");
        
        float pfLevel;
        hr = pSimpleAudioVolume->GetMasterVolume(&pfLevel);
        EXIT_ON_ERROR(hr, "GetMasterVolume error");
        std::cout << "Volume: " << pfLevel << std::endl;

        hr = pSimpleAudioVolume->SetMasterVolume(0.3, NULL);
        EXIT_ON_ERROR(hr, "SetMasterVolume error");

        hr = pSimpleAudioVolume->GetMasterVolume(&pfLevel);
        EXIT_ON_ERROR(hr, "GetMasterVolume error");
        std::cout << "Volume: " << pfLevel << std::endl;


        IAudioSessionControl2* sessionControl2 = NULL;
        sessionControl->QueryInterface(__uuidof(sessionControl2), (void**)&sessionControl2);
        EXIT_ON_ERROR(hr, "QueryInterface error");


        hr = sessionControl2->IsSystemSoundsSession();
        EXIT_ON_ERROR(hr, "IsSystemSoundsSession error");
        if (hr == S_OK)
            std::cout << "==System sound==" << std::endl;;

        DWORD* processId = NULL;
        hr = sessionControl2->GetProcessId((DWORD*)&processId);
        EXIT_ON_ERROR(hr, "GetProcessId error");
        std::cout << "Process id: " << processId << std::endl;



        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)processId);
        if (hProcess != NULL) {
            char buffer[MAX_PATH];
            GetProcessImageFileNameA(hProcess, buffer, MAX_PATH);
            std::string result = "UNKNOWN";
            result = std::string(buffer);
            CloseHandle(hProcess);
            std::cout << "result: " << result << std::endl;
        }


        sessionControl2->Release();
        pSimpleAudioVolume->Release();
        sessionControl->Release();
        

    }

    sessionEnum->Release();
    CoUninitialize();
	return 0;

}
