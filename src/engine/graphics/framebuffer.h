#pragma once
#include <glad/glad.h>
#include <memory>

namespace Karbon {

class Framebuffer {

public:
    Framebuffer(uint32_t width, uint32_t height);
    ~Framebuffer();

    void bind();
    void unbind();
    void resize(uint32_t width, uint32_t height);

    GLuint getColorAttachment() const { return m_colorAttachment; }
    GLuint getDepthAttachment() const { return m_depthAttachment; }
    GLuint getID() const { return m_ID; }
    unsigned int getWidth() const { return m_width; }
    unsigned int getHeight() const { return m_height; }
private:
    GLuint m_ID;
    GLuint m_colorAttachment;
    GLuint m_depthAttachment;
    unsigned int m_width, m_height;
};

}