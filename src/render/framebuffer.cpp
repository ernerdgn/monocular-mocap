#include "framebuffer.hpp"
#include "core/logger.hpp"

namespace mocap {

Framebuffer::Framebuffer() = default;

Framebuffer::~Framebuffer() 
{
    cleanup();
}

void Framebuffer::cleanup() 
{
    if (m_fboID) {
        glDeleteFramebuffers(1, &m_fboID);
        glDeleteTextures(1, &m_textureID);
        glDeleteRenderbuffers(1, &m_rboID);
        m_fboID = 0;
        m_textureID = 0;
        m_rboID = 0;
    }
}

Result<void> Framebuffer::initialize(int width, int height) 
{
    m_width = width;
    m_height = height;

    cleanup(); // a fresh start

    // framebuffen obj
    glGenFramebuffers(1, &m_fboID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);

    // color attch. text.
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureID, 0);

    // depth render buffer
    glGenRenderbuffers(1, &m_rboID);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rboID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rboID);

    // verify
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return Result<void>("Framebuffer is not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return Result<void>();
}

void Framebuffer::resize(int width, int height) 
{
    if (width == m_width && height == m_height) return;
    if (width <= 0 || height <= 0) return;
    
    initialize(width, height);
}

void Framebuffer::bind() 
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
    glViewport(0, 0, m_width, m_height);
}

void Framebuffer::unbind() 
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}