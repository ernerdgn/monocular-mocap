#include "core/logger.hpp"
#include "capture/camera.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <chrono>
#include <string>

int main()
{
    mocap::Logger::Init();
    MOCAP_INFO("MoCap System Starting...");

    mocap::Camera cam;
    
    auto open_result = cam.Open(0);
    if (!open_result.is_ok())
    {
        MOCAP_CRITICAL("Shutting down: {}", open_result.error());
        return -1;
    }

    MOCAP_INFO("Press 'ESC' in the video window to quit.");

    // fps track
    int frame_count = 0;
    float fps = 0.0f;
    auto time_start = std::chrono::high_resolution_clock::now();

    while (true)
    {
        auto frame_result = cam.ReadFrame();
        
        if (!frame_result.is_ok())
        {
            MOCAP_ERROR("Frame drop: {}", frame_result.error());
            break;
        }

        cv::Mat frame = frame_result.value();

        // get fps
        frame_count++;
        auto time_now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = time_now - time_start;
        
        // update every 1s
        if (elapsed.count() >= 1.0f)
        {
            fps = frame_count / elapsed.count();
            frame_count = 0;
            time_start = time_now;
        }

        // draw fps
        std::string fps_text = "FPS: " + std::to_string(static_cast<int>(fps));
        cv::putText(frame, fps_text, cv::Point(10, 30), 
                    cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);

        cv::imshow("Monocular MoCap - Raw Feed", frame);

        if (cv::waitKey(1) == 27)
        {
            MOCAP_INFO("ESC pressed. Exiting loop.");
            break;
        }
    }

    cam.Close();
    cv::destroyAllWindows();
    MOCAP_INFO("System shutdown complete.");
    return 0;
}