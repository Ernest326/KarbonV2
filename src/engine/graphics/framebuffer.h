#pragma once
#include <glad/glad.h>
#include <memory>

namespace Karbon {

class Framebuffer {

public:
    Framebuffer(uint32_t width, uint32_t height);
    ~Framebuffer();

    // Owns GL framebuffer + attachment handles: non-copyable, movable
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&& other) noexcept
        : m_ID(other.m_ID), m_colorAttachment(other.m_colorAttachment),
          m_depthAttachment(other.m_depthAttachment),
          m_width(other.m_width), m_height(other.m_height) {
        other.m_ID = 0;
        other.m_colorAttachment = 0;
        other.m_depthAttachment = 0;
    }
    Framebuffer& operator=(Framebuffer&& other) noexcept {
        if (this != &other) {
            glDeleteFramebuffers(1, &m_ID);
            glDeleteTextures(1, &m_colorAttachment);
            glDeleteTextures(1, &m_depthAttachment);
            m_ID = other.m_ID;
            m_colorAttachment = other.m_colorAttachment;
            m_depthAttachment = other.m_depthAttachment;
            m_width = other.m_width;
            m_height = other.m_height;
            other.m_ID = 0;
            other.m_colorAttachment = 0;
            other.m_depthAttachment = 0;
        }
        return *this;
    }

    void bind();
    void unbind();
    void resize(uint32_t width, uint32_t height);

    GLuint getColorAttachment() const { return m_colorAttachment; }
    GLuint getDepthAttachment() const { return m_depthAttachment; }
    GLuint getID() const { return m_ID; }
    unsigned int getWidth() const { return m_width; }
    unsigned int getHeight() const { return m_height; }
private:
    GLuint m_ID = 0;
    GLuint m_colorAttachment = 0;
    GLuint m_depthAttachment = 0;
    unsigned int m_width = 0, m_height = 0;
};

}