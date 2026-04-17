#pragma once
#include <opencv2/opencv.hpp>
// #include <opencv4/opencv2/opencv.hpp>

namespace mocap
{

class Texture
{
  public:
    Texture();
    ~Texture();

    // prevent copy
    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;

    // allow move
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    // upload from cpu to gpu
    void update(const cv::Mat& frame);

    unsigned int getId() const
    {
        return m_rendererId;
    }
    int getWidth() const
    {
        return m_width;
    }
    int getHeight() const
    {
        return m_height;
    }
    bool isValid() const
    {
        return m_rendererId != 0;
    }

  private:
    unsigned int m_rendererId;
    int m_width;
    int m_height;
};

} // namespace mocap