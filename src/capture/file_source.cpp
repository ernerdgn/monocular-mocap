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

    Result<cv::Mat> FileSource::readFrame()
    {
        if (!m_capture.isOpened()) return Result<cv::Mat>::err("Source not open");
        
        cv::Mat frame;
        m_capture >> frame;
        
        if (frame.empty()) return Result<cv::Mat>::err("EOF or empty frame");
        
        return Result<cv::Mat>::ok(frame);
    }

    double FileSource::getFPS() const { return m_capture.get(cv::CAP_PROP_FPS); }
    int FileSource::getWidth() const { return (int)m_capture.get(cv::CAP_PROP_FRAME_WIDTH); }
    int FileSource::getHeight() const { return (int)m_capture.get(cv::CAP_PROP_FRAME_HEIGHT); }

}