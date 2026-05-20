#include "vao.h"

namespace Karbon {

VAO::VAO() {
    glGenVertexArrays(1, &m_rendererID);
}

VAO::~VAO() {
    glDeleteVertexArrays(1, &m_rendererID);
}

void VAO::bind() const {
    glBindVertexArray(m_rendererID);
}

void VAO::unbind() const {
    glBindVertexArray(0);
}

void VAO::addBuffer(const VBO& vbo, unsigned int index, unsigned int size, unsigned int type, bool normalized, unsigned int stride, unsigned int offset) {
    bind();
    vbo.bind();
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, size, type, normalized, stride, (const void*)offset);
}

}