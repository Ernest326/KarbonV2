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

    void bind() const;
    void unbind() const;

    template<typename T>
    void bindUniform(const T& value, const char* name);

    GLuint getID() const { return m_shaderID; }

private:
    GLuint m_shaderID;
};

}