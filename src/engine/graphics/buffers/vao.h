#pragma once
#include "vbo.h"

namespace Karbon {

class VAO {
public:
    VAO();
    ~VAO();

    void bind() const;
    void unbind() const;
    GLuint getID() const { return m_rendererID; }

    void addBuffer(const VBO& vbo, unsigned int index, unsigned int size, unsigned int type, bool normalized, unsigned int stride, unsigned int offset);

private:
    GLuint m_rendererID;

};

}