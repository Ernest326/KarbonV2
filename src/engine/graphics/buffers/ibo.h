#pragma once
#include <glad/glad.h>

namespace Karbon {

class IBO {
public:
    IBO(const unsigned int* data, unsigned int count);
    ~IBO();

    void bind() const;
    void unbind() const;

    GLuint getID() const { return m_rendererID; }
    unsigned int getCount() const { return m_count; }

private:
    GLuint m_rendererID;
    unsigned int m_count;
};

}