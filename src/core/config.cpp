#include "config.hpp"

#include <nlohmann/json.hpp>

#include <fstream>

namespace mocap
{

Result<Config> Config::load(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        return Result<Config>("Could not open config file: " + path);

    try
    {
        nlohmann::json j;
        file >> j;

        Config cfg;
        cfg.camera.device_id  = j.value("/camera/device_id"_json_pointer, 0);
        cfg.camera.width      = j.value("/camera/width"_json_pointer, 1280);
        cfg.camera.height     = j.value("/camera/height"_json_pointer, 720);
        cfg.camera.target_fps = j.value("/camera/target_fps"_json_pointer, 30);
        
        cfg.smplx_path     = j.value("/model_paths/smplx_onnx"_json_pointer, "smplx.onnx");
        cfg.detection_path = j.value("/model_paths/detection_onnx"_json_pointer, "yolov8n-pose.onnx");

        return Result<Config>(cfg);
    }
    catch (const std::exception& e)
    {
        return Result<Config>(std::string("JSON Parse Error: ") + e.what());
    }
}

}