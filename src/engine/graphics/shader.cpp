#include "shader.h"
#include <fstream>

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

}

Shader::~Shader() {
    glDeleteProgram(m_shaderID);
}

void Shader::bind() const {
    glUseProgram(m_shaderID);
}

void Shader::unbind() const {
    glUseProgram(0);
}

template<typename T>
void Shader::bindUniform(const T& value, const char* name) {
    if(std::is_same<T, int>::value) {
        glUniform1i(glGetUniformLocation(m_shaderID, name), value);
    } else if(std::is_same<T, float>::value) {
        glUniform1f(glGetUniformLocation(m_shaderID, name), value);
    } else if(std::is_same<T, glm::vec3>::value) {
        glUniform3fv(glGetUniformLocation(m_shaderID, name), 1, &value[0]);
    } else if(std::is_same<T, glm::vec4>::value) {
        glUniform4fv(glGetUniformLocation(m_shaderID, name), 1, &value[0]);
    } else if(std::is_same<T, glm::mat4>::value) {
        glUniformMatrix4fv(glGetUniformLocation(m_shaderID, name), 1, GL_FALSE, &value[0][0]);
    } else {
        std::cout << "ERROR: UNSUPPORTED UNIFORM TYPE" << std::endl;
    }
}

}