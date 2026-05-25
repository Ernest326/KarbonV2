#pragma once
#include "mesh.h"

namespace Karbon {

class CubeMesh : public Mesh {
public:
    CubeMesh()
    {
        this->vertices = {
            Vertex(glm::vec3(-1.0f,-1.0f,-1.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec2(0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f)),
            Vertex(glm::vec3(1.0f,-1.0f,-1.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec2(0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f)),
            Vertex(glm::vec3(1.0f,1.0f,-1.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec2(0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f)),
            Vertex(glm::vec3(-1.0f,1.0f,-1.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec2(0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f)),
            Vertex(glm::vec3(-1.0f,-1.0f,1.0f),glm::vec3(0.0f,0.0f,1.0f),glm::vec2(0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f)),
            Vertex(glm::vec3(1.0f,-1.0f,1.0f),glm::vec3(0.0f,0.0f,1.0f),glm::vec2(0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f)),
            Vertex(glm::vec3(1.0f,1.0f,1.0f),glm::vec3(0.0f,0.0f,1.0f),glm::vec2(0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f)),
            Vertex(glm::vec3(-1.0f,1.0f,1.0f),glm::vec3(0.0f,0.0f,1.0f),glm::vec2(0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f))
        };
        this->indices = {
            0, 1, 2,
            0, 2, 3,
            4, 5, 6,
            4, 6, 7,
            0, 4, 7,
            0, 7, 3,
            1, 5, 6,
            1, 6, 2,
            2, 6, 7,
            2, 7, 3,
            0, 4, 5,
            0, 5, 1
        };
        
        uploadToGPU();
    }
};

}