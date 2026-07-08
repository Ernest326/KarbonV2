#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

namespace Karbon {

class Shader {
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    // Owns a GL program handle: non-copyable, movable
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept : m_shaderID(other.m_shaderID) { other.m_shaderID = 0; }
    Shader& operator=(Shader&& other) noexcept {
        if (this != &other) {
            release();
            m_shaderID = other.m_shaderID;
            other.m_shaderID = 0;
        }
        return *this;
    }

    void bind() const;
    void unbind() const;

    template<typename T>
    void bindUniform(const T& value, const char* name);

    GLuint getID() const { return m_shaderID; }

private:
    void release() {
        if (m_shaderID) {
            glDeleteProgram(m_shaderID);
            m_shaderID = 0;
        }
    }

    GLuint m_shaderID = 0;
};

}