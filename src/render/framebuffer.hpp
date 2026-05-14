#pragma once
#include <glad/glad.h>
#include "core/result.hpp"

namespace mocap {

class Framebuffer
{
public:
    Framebuffer();
    ~Framebuffer();

    Result<void> initialize(int width, int height);

    void resize(int width, int height);

    void bind();
    void unbind();

    GLuint getTextureID() const { return m_textureID; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    void cleanup();

    GLuint m_fboID = 0;
    GLuint m_textureID = 0;
    GLuint m_rboID = 0; // rb for depth test
    int m_width = 0;
    int m_height = 0;
};

}