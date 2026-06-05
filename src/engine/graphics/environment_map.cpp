#include "environment_map.h"
#include <stb_image/stb_image.h>
#include <iostream>
#include <glad/glad.h>
#include "../graphics/primitives/cube.h"

namespace Karbon {

VAO* EnvironmentMap::m_envVAO = nullptr;
IBO* EnvironmentMap::m_envIBO = nullptr;

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
    if (m_irradianceMap) glDeleteTextures(1, &m_irradianceMap);
    if (m_prefilterMap) glDeleteTextures(1, &m_prefilterMap);
    if (m_brdfLUT) glDeleteTextures(1, &m_brdfLUT);
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
    Cube cube;
    m_envVAO = new VAO();
    m_envVAO->bind();
    m_envIBO = &cube.getIndexBuffer();
    m_envIBO->bind();
    m_envVAO->addBuffer(cube.getVertexBuffer(), 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    m_envVAO->unbind();
}

void EnvironmentMap::convertToCubemap() {
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

    //Cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);
    glDeleteTextures(1, &m_equirectTexture);
    m_equirectTexture = 0;
}

// ------------------------------------------------------------------
// IBL generation (identical pipeline to Cubemap, using m_cubemap as source)
// ------------------------------------------------------------------
void EnvironmentMap::generateIBL() {
    if (m_hasIBL) return;

    Shader irradianceShader("resources/shaders/compute/ibl.vert", "resources/shaders/compute/irradiance.frag");
    Shader prefilterShader("resources/shaders/compute/ibl.vert", "resources/shaders/compute/prefilter.frag");
    Shader brdfShader("resources/shaders/compute/brdf.vert", "resources/shaders/compute/brdf.frag");

    GLuint captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // --- Irradiance ---
    glGenTextures(1, &m_irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // --- Prefilter ---
    glGenTextures(1, &m_prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterMap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // --- BRDF LUT ---
    glGenTextures(1, &m_brdfLUT);
    glBindTexture(GL_TEXTURE_2D, m_brdfLUT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 256, 256, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    if (!m_envVAO)
        createVAO();

    // --- Irradiance pass ---
    irradianceShader.bind();
    irradianceShader.bindUniform(0, "environmentMap");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glViewport(0, 0, 32, 32);
    for (unsigned int i = 0; i < 6; ++i) {
        irradianceShader.bindUniform(captureViews[i], "view");
        irradianceShader.bindUniform(captureProjection, "projection");
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_envVAO->bind();
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

    // --- Prefilter pass ---
    prefilterShader.bind();
    prefilterShader.bindUniform(0, "environmentMap");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    GLuint maxMipLevels = 5;
    for (GLuint mip = 0; mip < maxMipLevels; ++mip) {
        unsigned int mipWidth = 128 >> mip;
        unsigned int mipHeight = 128 >> mip;
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader.bindUniform(roughness, "roughness");
        prefilterShader.bindUniform(captureProjection, "projection");

        for (unsigned int i = 0; i < 6; ++i) {
            prefilterShader.bindUniform(captureViews[i], "view");
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_prefilterMap, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            m_envVAO->bind();
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
    }

    // --- BRDF LUT pass (fullscreen quad) ---
    {
        float quadVerts[] = {
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        GLuint quadVAO, quadVBO;
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        brdfShader.bind();
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdfLUT, 0);
        glViewport(0, 0, 256, 256);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindVertexArray(0);
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
    }

    // --- Cleanup ---
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);

    m_hasIBL = true;
    }
}