#include "config.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

namespace mocap {

    Result<Config> Config::load(const std::string& path)
    {
        std::ifstream file(path);
        if (!file.is_open()) return Result<Config>("Could not open config file: " + path);

        try 
        {
            nlohmann::json j;
            file >> j;

            Config cfg;
            cfg.camera.device_id = j["camera"]["device_id"];
            cfg.camera.width = j["camera"]["width"];
            cfg.camera.height = j["camera"]["height"];
            cfg.camera.target_fps = j["camera"]["target_fps"];
            cfg.smplx_path = j["model_paths"]["smplx_onnx"];
            cfg.detection_path = j["model_paths"]["detection_onnx"];

            return Result<Config>(cfg);
        }
        catch (const std::exception& e)
        {
            return Result<Config>(std::string("JSON Parse Error: ") + e.what());
        }
    }

}