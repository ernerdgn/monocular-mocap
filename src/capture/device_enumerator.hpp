#pragma once
#include <vector>
#include <string>

namespace mocap {

    struct CameraDevice
    {
        int id;
        std::string name;
    };

    class DeviceEnumerator
    {
    public:
        // interrogates the os for connected camera hardware
        static std::vector<CameraDevice> getAvailableCameras();
    };

}