#include "camera.hpp"

namespace mocap {

    Camera::Camera() : m_isOpen(false), m_isRunning(false), m_frameQueue(2) {} // Queue holds max 2 frames

    Camera::~Camera()
    {
        close();
    }

    Result<void> Camera::open(int device_id)
    {
        MOCAP_INFO("Starting cam, device id: {}", device_id);
        m_capture.open(device_id, cv::CAP_ANY);

        if (!m_capture.isOpened())
        {
            MOCAP_ERROR("Failed to open cam, id: {}", device_id);
            return Result<void>::fail("cant open cam.");
        }

        // force standard res
        m_capture.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        m_capture.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

        m_isOpen = true;
        m_isRunning = true;

        // launch bg thread
        m_workerThread = std::jthread(&Camera::captureLoop, this);

        MOCAP_INFO("Cam ({}) opened and background thread started!", device_id);
        return Result<void>::ok();
    }

    void Camera::close()
    {
        if (m_isOpen)
        {
            m_isRunning = false;
            m_frameQueue.close();
            
            // wait for thread
            if (m_workerThread.joinable()) m_workerThread.join();

            m_capture.release();
            m_isOpen = false;
            MOCAP_INFO("Cam closed.");
        }
    }

    void Camera::captureLoop()
    {
        while (m_isRunning)
        {
            cv::Mat frame;
            m_capture >> frame;

            if (frame.empty()) continue;

            // try push
            if (!m_frameQueue.try_push(frame))
            {
                m_frameQueue.try_pop();
                m_frameQueue.try_push(frame);
            }
        }
    }

    std::optional<cv::Mat> Camera::getLatestFrame()
    {
        if (!m_isOpen) return std::nullopt;

        std::optional<cv::Mat> latest;
        std::optional<cv::Mat> current;

        // drain queue
        while ((current = m_frameQueue.try_pop()).has_value()) latest = current;

        return latest;
    }

}