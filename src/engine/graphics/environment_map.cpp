#include "environment_map.h"
#include "graphics/shader.h"
#include <stb_image/stb_image.h>
#include <iostream>
#include <glad/glad.h>

namespace Karbon {

VAO* EnvironmentMap::m_envVAO = nullptr;
IBO* EnvironmentMap::m_envIBO = nullptr;
Cube* EnvironmentMap::m_envCube = nullptr;

EnvironmentMap::EnvironmentMap(const std::string &hdrPath) {
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    float *data = stbi_loadf(hdrPath.c_str(), &width, &height, &nrChannels, 0);

    if(!data) {
        std::cout << "Failed to load HDR image: " << hdrPath << std::endl;
        stbi_set_flip_vertically_on_load(false);
        return;
    }

    glGenTextures(1, &m_equirectTexture);
    glBindTexture(GL_TEXTURE_2D, m_equirectTexture);

    GLenum internalFormat = GL_RGB16F;
    GLenum dataFormat = GL_RGB;
    if (nrChannels == 1) { internalFormat = GL_R16F; dataFormat = GL_RED; }
    else if (nrChannels == 3) { internalFormat = GL_RGB16F; dataFormat = GL_RGB; }
    else if (nrChannels == 4) { internalFormat = GL_RGBA16F; dataFormat = GL_RGBA; }

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(false);

    convertToCubemap();

}

EnvironmentMap::~EnvironmentMap() {
    if (m_cubemap) glDeleteTextures(1, &m_cubemap);
    if (m_equirectTexture) glDeleteTextures(1, &m_equirectTexture);
}

void EnvironmentMap::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap);
    m_envVAO->bind();
}

void EnvironmentMap::unbind() const {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    m_envVAO->unbind();
}

void EnvironmentMap::createVAO() {
    m_envCube = new Cube();
    m_envVAO = new VAO();
    m_envVAO->bind();
    m_envVAO->addBuffer(m_envCube->getVertexBuffer(), 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    m_envCube->getIndexBuffer().bind();
    m_envVAO->unbind();
}

void EnvironmentMap::convertToCubemap() {
    // FIX: Save and neutralize GL state that breaks offscreen cube capture.
    // The capture camera sits INSIDE the unit cube, so back-face culling
    // discards every face and leaves the render target at the clear color
    // (solid black faces). Blend/scissor leaking in from the scene renderer
    // corrupts the output the same way.
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    GLboolean prevCull    = glIsEnabled(GL_CULL_FACE);
    GLboolean prevBlend   = glIsEnabled(GL_BLEND);
    GLboolean prevScissor = glIsEnabled(GL_SCISSOR_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    // FIX: Without seamless filtering, sampling across cube-face edges does not
    // interpolate between faces -> dark seam lines along every cube edge in the
    // IBL maps. Safe to leave enabled globally.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Create cubemap texture
    glGenTextures(1, &m_cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLuint captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Cubemap conversion FBO incomplete!" << std::endl;
    }

    Shader equirectShader("resources/shaders/compute/ibl.vert", "resources/shaders/compute/equirect_to_cubemap.frag");

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
    };

    if(!m_envVAO) {
        createVAO();
    }

    equirectShader.bind();
    equirectShader.bindUniform(0, "equirectangularMap");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_equirectTexture);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glViewport(0, 0, 512, 512);
    for (unsigned int i = 0; i < 6; ++i) {
        equirectShader.bindUniform(captureProjection, "projection");
        equirectShader.bindUniform(captureViews[i], "view");
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_cubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_envVAO->bind();
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    // FIX: Switch to a mipmap-aware filter AFTER generation.
    // GL_LINEAR (set above) marks the sampler as non-mipmapped — drivers will
    // ignore the LOD argument in textureLod() or return black when mip > 0.
    // GL_LINEAR_MIPMAP_LINEAR enables trilinear mip traversal so the explicit
    // LOD bias computed in prefilter.frag actually selects the correct mip level.
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    //Cleanup
    m_envVAO->unbind(); // FIX: don't leak the env VAO (and its element buffer binding) into the scene
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);
    glDeleteTextures(1, &m_equirectTexture);
    m_equirectTexture = 0;

    // FIX: Restore caller GL state (viewport was left at 512x512 before).
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
    if (prevCull)    glEnable(GL_CULL_FACE);
    if (prevBlend)   glEnable(GL_BLEND);
    if (prevScissor) glEnable(GL_SCISSOR_TEST);
}


}