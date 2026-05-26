#include "cubemap.h"
#include <stb_image/stb_image.h>
#include <iostream>
#include <string>
#include <glad/glad.h>
#include "primitives/cube.h"

namespace Karbon {

VAO* Cubemap::m_cubemapVAO = nullptr;
IBO* Cubemap::m_cubemapIBO = nullptr;

Cubemap::Cubemap(const std::vector<std::string> &faces) {
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
    if (faces.size() != 6) {
        std::cerr << "Warning: Cubemap expects 6 faces, got " << faces.size() << std::endl;
    }

    // Ensure images are not flipped for cubemap faces
    stbi_set_flip_vertically_on_load(false);

    for (unsigned int i = 0; i < faces.size() && i < 6; i++) {
        int width, height, nrChannels;
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = GL_RGB;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


    if (!m_cubemapVAO)
        createVAO();
}

Cubemap::~Cubemap() {
    glDeleteTextures(1, &m_id);
}

void Cubemap::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
    // Bind VAO (which already has the element buffer bound) then ensure texture is active
    m_cubemapVAO->bind();
}

void Cubemap::unbind() const {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    // Unbind VAO only. Do not unbind the shared IBO globally (it's owned by Cube)
    m_cubemapVAO->unbind();
}

void Cubemap::createVAO() {
    // Create a Cube instance to obtain the shared static buffers.
    Cube cube; // uses defaults from header
    m_cubemapVAO = new VAO();
    // Bind VAO, set vertex attrib and bind the cube's index buffer to the VAO
    m_cubemapVAO->bind();
    m_cubemapVAO->addBuffer(cube.getVertexBuffer(), 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    // The Cube exposes a reference to a static IBO; store its address and bind it while VAO is bound
    m_cubemapIBO = &cube.getIndexBuffer();
    m_cubemapIBO->bind();
    m_cubemapVAO->unbind();

}

}