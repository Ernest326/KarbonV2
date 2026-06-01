#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "graphics/shader.h"

namespace Karbon {

class Grid {

public:
    Grid(float size = 1.0f, int divisions = 10);
    ~Grid() {};

    void Draw(glm::mat4 view, glm::mat4 projection);

private:
    static GLuint gridVAO, gridVBO;
    static Shader* gridShader;
    float m_size;
    int m_divisions;
};
}