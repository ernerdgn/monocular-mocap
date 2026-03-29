#include "texture.hpp"
#include <glad/glad.h>

namespace mocap {
    Texture::Texture() : m_rendererId(0), m_width(0), m_height(0) {}

    Texture::~Texture()
    {
        if (m_rendererId != 0) glDeleteTextures(1, &m_rendererId);
    }

    Texture::Texture(Texture&& other) noexcept 
        : m_rendererId(other.m_rendererId), m_width(other.m_width), m_height(other.m_height)
    {
        other.m_rendererId = 0;
    }

    Texture& Texture::operator=(Texture&& other) noexcept
    {
        if (this != &other)
        {
            if (m_rendererId != 0) glDeleteTextures(1, &m_rendererId);
            m_rendererId = other.m_rendererId;
            m_width = other.m_width;
            m_height = other.m_height;
            other.m_rendererId = 0;
        }
        return *this;
    }

    void Texture::update(const cv::Mat& frame)
    {
        if (frame.empty()) return;

        // cv def BGR!.. why?
        GLenum dataFormat = (frame.channels() == 3) ? GL_BGR : GL_BGRA;
        GLenum internalFormat = (frame.channels() == 3) ? GL_RGB : GL_RGBA;

        if (m_rendererId == 0)
        {
            glGenTextures(1, &m_rendererId);
            glBindTexture(GL_TEXTURE_2D, m_rendererId);
            
            // texture scaling filters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        else
        {
            glBindTexture(GL_TEXTURE_2D, m_rendererId);
        }

        // ensure correct byte alignment for image rows
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        
        if (m_width != frame.cols || m_height != frame.rows)
        {
            // allocate new GPU memory if the resolution changed
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, frame.cols, frame.rows, 0, dataFormat, GL_UNSIGNED_BYTE, frame.data);
            m_width = frame.cols;
            m_height = frame.rows;
        }
        else
        {
            // just overwrite existing GPU memory
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.cols, frame.rows, dataFormat, GL_UNSIGNED_BYTE, frame.data);
        }
    }
}
