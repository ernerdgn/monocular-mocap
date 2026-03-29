#include "device_enumerator.hpp"

#ifdef _WIN32
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
// hey msvc, automatically link the media libraries please
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#else
#include <opencv2/opencv.hpp>
#endif

namespace mocap {

std::vector<CameraDevice> DeviceEnumerator::getAvailableCameras()
{
    std::vector<CameraDevice> cameras;

#ifdef _WIN32
    // COM init
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    bool coInit = SUCCEEDED(hr);

    IMFAttributes* pAttributes = NULL;
    hr = MFCreateAttributes(&pAttributes, 1);
    
    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, 
                                  MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    }

    IMFActivate** ppDevices = NULL;
    UINT32 count = 0;
    if (SUCCEEDED(hr))
    {
        hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);
    }

    if (SUCCEEDED(hr))
    {
        for (UINT32 i = 0; i < count; i++)
        {
            WCHAR* szFriendlyName = NULL;
            UINT32 cchName;
            // extract hardware name
            hr = ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, 
                                                  &szFriendlyName, &cchName);
            if (SUCCEEDED(hr))
            {
                int size_needed = WideCharToMultiByte(CP_UTF8, 0, szFriendlyName, -1, NULL, 0, NULL, NULL);
                std::string name(size_needed - 1, 0); // -1 removes the trailing null character
                WideCharToMultiByte(CP_UTF8, 0, szFriendlyName, -1, &name[0], size_needed, NULL, NULL);

                cameras.push_back({ (int)i, name });
                CoTaskMemFree(szFriendlyName);
            }
            ppDevices[i]->Release();
        }
        CoTaskMemFree(ppDevices);
    }

    if (pAttributes) pAttributes->Release();
    if (coInit) CoUninitialize();

#else
    // fallback for Linux: probe the first 4 indices using cv
    for (int i = 0; i < 4; i++)
    {
        cv::VideoCapture cap(i, cv::CAP_ANY);
        if (cap.isOpened())
        {
            cameras.push_back({ i, "Camera Device " + std::to_string(i) });
        }
    }
#endif

    // abs fallback if nothing is found
    if (cameras.empty())
    {
        cameras.push_back({0, "Default Camera"});
    }

    return cameras;
}

}