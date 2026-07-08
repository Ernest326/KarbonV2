#include "grid.h"

namespace Karbon {
GLuint Grid::gridVAO = 0;
GLuint Grid::gridVBO = 0;
Shader* Grid::gridShader = nullptr;

Grid::Grid(float size, int divisions) {
    this->m_size = size;
    this->m_divisions = divisions;
}

void Grid::draw(glm::mat4 view, glm::mat4 projection) {
    if (gridVAO == 0) {

        gridShader = new Shader("resources/shaders/grid.vert", "resources/shaders/grid.frag");

        std::vector<glm::vec3> lines;
        const float half = m_size * 0.5f;
        const float step = m_size / m_divisions;
        for (int i = 0; i<= m_divisions; ++i) {
            float p = -half + i * step;
            lines.push_back(glm::vec3(p, 0, -half));
            lines.push_back(glm::vec3(p, 0, half));
            lines.push_back(glm::vec3(-half, 0, p));
            lines.push_back(glm::vec3(half, 0, p));
        }
        glGenVertexArrays(1, &gridVAO);
        glGenBuffers(1, &gridVBO);
        glBindVertexArray(gridVAO);
        glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
        glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(glm::vec3), lines.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glBindVertexArray(0);
    }

    glBindVertexArray(gridVAO);
    gridShader->bind();
    gridShader->bindUniform(projection * view * glm::mat4(1.0f), "u_MVP");
    glDrawArrays(GL_LINES, 0, (m_divisions + 1) * 4);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

}
}