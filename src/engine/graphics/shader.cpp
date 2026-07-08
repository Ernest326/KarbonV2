#include "shader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <glm/glm.hpp>

namespace Karbon {

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cout << "ERROR: SHADER FILE NOT SUCCESSFULLY READ" << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR: VERTEX SHADER COMPILATION FAILED\n" << infoLog << std::endl;
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "ERROR: FRAGMENT SHADER COMPILATION FAILED\n" << infoLog << std::endl;
    }

    m_shaderID = glCreateProgram();
    glAttachShader(m_shaderID, vertex);
    glAttachShader(m_shaderID, fragment);
    glLinkProgram(m_shaderID);

    glGetProgramiv(m_shaderID, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(m_shaderID, 512, NULL, infoLog);
        std::cout << "ERROR: SHADER PROGRAM LINKING FAILED\n" << infoLog << std::endl;
    }

    // shaders are linked into program; we can delete the shader objects
    glDeleteShader(vertex);
    glDeleteShader(fragment);

}

Shader::~Shader() {
    release();
}

void Shader::bind() const {
    glUseProgram(m_shaderID);
}

void Shader::unbind() const {
    glUseProgram(0);
}

GLint Shader::getUniformLocation(const char* name) {
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) {
        return it->second;
    }
    GLint location = glGetUniformLocation(m_shaderID, name);
    m_uniformCache.emplace(name, location);
    return location;
}

//bindUniform(value, name) - Detect type and bind it to correct uniform function
template<typename T>
void Shader::bindUniform(const T& value, const char* name) {
    if constexpr (std::is_same_v<T, int>) {
        glUniform1i(getUniformLocation(name), value);
    } else if constexpr (std::is_same_v<T, float>) {
        glUniform1f(getUniformLocation(name), value);
    } else if constexpr (std::is_same_v<T, glm::vec3>) {
        glUniform3fv(getUniformLocation(name), 1, &value[0]);
    } else if constexpr (std::is_same_v<T, glm::vec4>) {
        glUniform4fv(getUniformLocation(name), 1, &value[0]);
    } else if constexpr (std::is_same_v<T, glm::mat4>) {
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    } else {
        static_assert(!std::is_same_v<T, T>, "ERROR: UNSUPPORTED UNIFORM TYPE");
    }
}

// Explicit instantiations for commonly used uniform types so the template
// defined in this translation unit is available to callers.
template void Shader::bindUniform<int>(const int& , const char*);
template void Shader::bindUniform<float>(const float& , const char*);
template void Shader::bindUniform<glm::vec3>(const glm::vec3& , const char*);
template void Shader::bindUniform<glm::vec4>(const glm::vec4& , const char*);
template void Shader::bindUniform<glm::mat4>(const glm::mat4& , const char*);

}

