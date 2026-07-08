#pragma once
#include <glad/glad.h>
#include <stb_image/stb_image.h>

namespace Karbon {
class Texture {
public:
    Texture(const char* path);
    ~Texture();

    // Owns a GL texture handle: non-copyable, movable
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept : m_textureID(other.m_textureID) { other.m_textureID = 0; }
    Texture& operator=(Texture&& other) noexcept {
        if (this != &other) {
            glDeleteTextures(1, &m_textureID);
            m_textureID = other.m_textureID;
            other.m_textureID = 0;
        }
        return *this;
    }

    void bind(unsigned int slot = 0) const;
    void unbind() const;

    GLuint getID() const { return m_textureID; }
private:
    GLuint m_textureID = 0;
};

}