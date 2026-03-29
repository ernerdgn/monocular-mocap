#include "camera_source.hpp"
#include "core/logger.hpp"

namespace mocap {

    CameraSource::CameraSource(int deviceId) : m_deviceId(deviceId) {}

    CameraSource::~CameraSource() { close(); }

    Result<void> CameraSource::open()
    {
        MOCAP_INFO("Starting CameraSource on device {}", m_deviceId);
        m_capture.open(m_deviceId, cv::CAP_ANY);

        if (!m_capture.isOpened())
        {
            return Result<void>::fail("Failed to open camera device.");
        }

        // force standard resolution
        m_capture.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        m_capture.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

        return Result<void>::ok();
    }

    void CameraSource::close()
    {
        if (m_capture.isOpened())
        {
            m_capture.release();
            MOCAP_INFO("CameraSource closed.");
        }
    }

    bool CameraSource::isOpen() const { return m_capture.isOpened(); }

    Result<cv::Mat> CameraSource::readFrame()
    {
        if (!m_capture.isOpened()) return Result<cv::Mat>::err("Source not open");
        
        cv::Mat frame;
        m_capture >> frame;
        if (frame.empty()) return Result<cv::Mat>::err("Empty frame");
        
        return Result<cv::Mat>::ok(frame);
    }

    double CameraSource::getFPS() const { return m_capture.get(cv::CAP_PROP_FPS); }
    int CameraSource::getWidth() const { return (int)m_capture.get(cv::CAP_PROP_FRAME_WIDTH); }
    int CameraSource::getHeight() const { return (int)m_capture.get(cv::CAP_PROP_FRAME_HEIGHT); }

}