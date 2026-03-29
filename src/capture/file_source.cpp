#include "file_source.hpp"
#include "core/logger.hpp"

namespace mocap {

FileSource::FileSource(const std::string& filePath) : m_filePath(filePath) {}

FileSource::~FileSource() { close(); }

Result<void> FileSource::open()
{
    MOCAP_INFO("Opening video file: {}", m_filePath);
    m_capture.open(m_filePath, cv::CAP_ANY);

    if (!m_capture.isOpened())
    {
        MOCAP_ERROR("Failed to open video file: {}", m_filePath);
        return Result<void>::fail("Failed to open video file.");
    }

    MOCAP_INFO("FileSource opened successfully ({}x{} @ {} FPS)", 
               getWidth(), getHeight(), getFPS());
    return Result<void>::ok();
}

void FileSource::close()
{
    if (m_capture.isOpened())
    {
        m_capture.release();
        MOCAP_INFO("FileSource closed.");
    }
}

bool FileSource::isOpen() const { return m_capture.isOpened(); }

Result<void> FileSource::readFrame(cv::Mat& outFrame)
{
    if (!m_capture.isOpened()) return Result<void>::fail("Source not open");
    
    // read directly into the provided pre-allocated memory
    m_capture >> outFrame;
    
    // when reading a file, an empty frame usually means End Of File (EOF)
    if (outFrame.empty()) return Result<void>::fail("EOF or empty frame");
    
    return Result<void>::ok();
}

double FileSource::getFPS() const { return m_capture.get(cv::CAP_PROP_FPS); }
int FileSource::getWidth() const { return (int)m_capture.get(cv::CAP_PROP_FRAME_WIDTH); }
int FileSource::getHeight() const { return (int)m_capture.get(cv::CAP_PROP_FRAME_HEIGHT); }

double FileSource::getVideoDuration() const
{
    if (!isOpen()) return 0.0;
    
    double totalFrames = m_capture.get(cv::CAP_PROP_FRAME_COUNT);
    double fps = getFPS();
    
    if (fps <= 0.0) return 0.0;
    return (totalFrames / fps) * 1000.0; // convert to milliseconds
}

Result<void> FileSource::seek(double timestampMs)
{
    if (!isOpen()) return Result<void>::fail("Source not open");
    
    // tell cv to jump to this specific millisecond
    bool success = m_capture.set(cv::CAP_PROP_POS_MSEC, timestampMs);
    
    if (!success) return Result<void>::fail("Failed to seek video file.");
    return Result<void>::ok();
}

}