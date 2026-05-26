#pragma once
#include <glad/glad.h>
#include <stb_image/stb_image.h>

namespace Karbon {
class Texture {
public:
    Texture(const char* path);
    ~Texture();

    void bind(unsigned int slot = 0) const;
    void unbind() const;

    GLuint getID() const { return m_textureID; }
private:
    GLuint m_textureID;
};

}