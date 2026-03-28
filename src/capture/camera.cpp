#include "camera.hpp"

namespace mocap {

    Camera::Camera() : m_isOpen(false) {}
    Camera::~Camera()
    {
        Close();
    }

    Result<void> Camera::Open(int device_id)
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
        MOCAP_INFO("Cam ({}) opened!", device_id);
        return Result<void>::ok();
    }

    void Camera::Close()
    {
        if (m_isOpen)
        {
            m_capture.release();
            m_isOpen = false;
            MOCAP_INFO("Cam closed.");
        }
    }

    Result<cv::Mat> Camera::ReadFrame()
    {
        if (!m_isOpen) return Result<cv::Mat>("Cam is not open.");

        cv::Mat frame;
        m_capture >> frame;

        if (frame.empty())
        {
            MOCAP_WARN("recieved empty frame from cam.");
            return Result<cv::Mat>("Empty frame.");
        }

        return Result<cv::Mat>(frame);
    }
}