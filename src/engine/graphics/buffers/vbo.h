#pragma once
#include <glad/glad.h>

namespace Karbon {

class VBO {
public:
    VBO(const void* data, unsigned int size);
    ~VBO();

    void bind() const;
    void unbind() const;

    GLuint getID() const { return m_rendererID; }

private:
    GLuint m_rendererID;
};

}