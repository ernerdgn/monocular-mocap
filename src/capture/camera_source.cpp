#include "camera_source.hpp"

#include "core/logger.hpp"

namespace mocap
{

CameraSource::CameraSource(int deviceId) : m_deviceId(deviceId)
{
}

CameraSource::~CameraSource()
{
    close();
}

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

bool CameraSource::isOpen() const
{
    return m_capture.isOpened();
}

Result<void> CameraSource::readFrame(cv::Mat& outFrame)
{
    if (!m_capture.isOpened()) return Result<void>::fail("Source not open");
    
    m_capture >> outFrame; // cv sees the pooled memory and overwrites it
    if (outFrame.empty()) return Result<void>::fail("Empty frame");
    
    return Result<void>::ok();
}

double CameraSource::getFPS() const
{
    return m_capture.get(cv::CAP_PROP_FPS);
}
int CameraSource::getWidth() const
{
    return (int) m_capture.get(cv::CAP_PROP_FRAME_WIDTH);
}
int CameraSource::getHeight() const
{
    return (int) m_capture.get(cv::CAP_PROP_FRAME_HEIGHT);
}

double CameraSource::getVideoDuration() const
{ 
    return 0.0; // live feed is infinite
}

Result<void> CameraSource::seek(double /*timestampMs*/)
{
    return Result<void>::fail("Cannot seek a live camera stream.");
}

} // namespace mocap