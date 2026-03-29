#pragma once
#include <string>
#include "core/result.hpp"

namespace mocap {

    struct CameraConfig
    {
        int device_id = 0;
        int width = 1280;
        int height = 720;
        int target_fps = 30;
    };

    struct Config
    {
        CameraConfig camera;
        std::string smplx_path;
        std::string detection_path;

        static Result<Config> load(const std::string& path);
    };

}