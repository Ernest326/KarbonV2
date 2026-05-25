#include "mesh.h"
#include <glm/gtc/constants.hpp>

namespace Karbon {

class SphereMesh : public Mesh {
public:
    SphereMesh(int stacks=16, int slices=16, float radius=0.5f) {
        for(unsigned int i=0; i<=stacks; ++i) {
            float phi = glm::pi<float>() * static_cast<float>(i) / static_cast<float>(stacks);
            for(unsigned int j=0; j<=slices; ++j) {
                float theta = 2.0f * glm::pi<float>() * static_cast<float>(j) / static_cast<float>(slices);
                Vertex vertex;
                vertex.position.x = std::cos(theta) * std::sin(phi) * radius;
                vertex.position.y = std::cos(phi) * radius;
                vertex.position.z = std::sin(theta) * std::sin(phi) * radius;
                vertex.normal = glm::normalize(vertex.position);
                vertex.texCoord.x = static_cast<float>(j) / static_cast<float>(slices);
                vertex.texCoord.y = static_cast<float>(i) / static_cast<float>(stacks);
                vertices.push_back(vertex);
            }
        }
        for(unsigned int i=0; i<stacks; ++i) {
            for(unsigned int j=0; j<slices; ++j) {
                unsigned int first = i * (slices + 1) + j;
                unsigned int second = first + slices + 1;
                
                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }
        uploadToGPU();
    }
};

}