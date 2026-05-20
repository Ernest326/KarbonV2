#include "vbo.h"

namespace Karbon {
    
VBO::VBO(const void* data, unsigned int size) {
    glGenBuffers(1, &m_rendererID);
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

VBO::~VBO() {
    glDeleteBuffers(1, &m_rendererID);
}

void VBO::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
}

void VBO::unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

}