#include "graphics/ibl_baker.h"
#include "graphics/shader.h"
#include "graphics/buffers/buffers.h"
#include "graphics/primitives/cube.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace Karbon {

namespace {

// Lazily built unit-cube VAO shared by all bake calls (positions + element buffer)
VAO* s_cubeVAO = nullptr;

void ensureCubeVAO() {
    if (s_cubeVAO) return;
    Cube cube; // exposes the shared static cube buffers
    s_cubeVAO = new VAO();
    s_cubeVAO->bind();
    s_cubeVAO->addBuffer(cube.getVertexBuffer(), 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    cube.getIndexBuffer().bind();
    s_cubeVAO->unbind();
}

}

void releaseIBLMaps(IBLMaps& maps) {
    if (maps.irradianceMap) glDeleteTextures(1, &maps.irradianceMap);
    if (maps.prefilterMap) glDeleteTextures(1, &maps.prefilterMap);
    if (maps.brdfLUT) glDeleteTextures(1, &maps.brdfLUT);
    maps = IBLMaps{};
}

IBLMaps bakeIBL(GLuint sourceCubemap, float sourceResolution) {
    IBLMaps maps;
    if (sourceCubemap == 0) {
        std::cerr << "Cannot bake IBL: missing source cubemap" << std::endl;
        return maps;
    }

    ensureCubeVAO();

    // Save and neutralize GL state that breaks offscreen cube capture.
    // The capture camera sits INSIDE the unit cube, so back-face culling
    // discards every face; blend/scissor leaking in from the scene renderer
    // corrupts the output the same way.
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    GLboolean prevCull    = glIsEnabled(GL_CULL_FACE);
    GLboolean prevBlend   = glIsEnabled(GL_BLEND);
    GLboolean prevScissor = glIsEnabled(GL_SCISSOR_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    // Without seamless filtering, sampling across cube-face edges does not
    // interpolate between faces -> dark seam lines along every cube edge.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // The prefilter pass samples source mips from the GGX PDF, so the source
    // must be mipmapped with a mip-aware min filter (GL_LINEAR marks the
    // sampler non-mipmapped and textureLod returns black for mip > 0).
    glBindTexture(GL_TEXTURE_CUBE_MAP, sourceCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    Shader irradianceShader("resources/shaders/compute/ibl.vert", "resources/shaders/compute/irradiance.frag");
    Shader prefilterShader("resources/shaders/compute/ibl.vert", "resources/shaders/compute/prefilter.frag");
    Shader brdfShader("resources/shaders/compute/brdf.vert", "resources/shaders/compute/brdf.frag");

    GLuint captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    const GLuint maxMipLevels = 5;

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // ========== IRRADIANCE PASS ==========
    glGenTextures(1, &maps.irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, maps.irradianceMap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    irradianceShader.bind();
    irradianceShader.bindUniform(0, "environmentMap");
    irradianceShader.bindUniform(captureProjection, "projection");

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
    glViewport(0, 0, 32, 32);

    for (unsigned int i = 0; i < 6; ++i) {
        irradianceShader.bindUniform(captureViews[i], "view");
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, maps.irradianceMap, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "ERROR: Irradiance FBO incomplete on face " << i << std::endl;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Re-bind the source cubemap on every face iteration.
        // glFramebufferTexture2D operates on GL_TEXTURE_CUBE_MAP targets and can
        // implicitly disturb the active cubemap binding on some drivers/platforms.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, sourceCubemap);

        s_cubeVAO->bind();
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

    // ========== PREFILTER PASS ==========
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0);

    glGenTextures(1, &maps.prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, maps.prefilterMap);
    for (unsigned int i = 0; i < 6; ++i) {
        unsigned int w = 128, h = 128;
        for (GLuint mip = 0; mip < maxMipLevels; ++mip) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mip, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, nullptr);
            w >>= 1;
            h >>= 1;
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, maxMipLevels - 1);

    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 128, 128);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    prefilterShader.bind();
    prefilterShader.bindUniform(0, "environmentMap");
    prefilterShader.bindUniform(captureProjection, "projection");
    // prefilter.frag selects a source mip per sample from the GGX PDF instead
    // of hammering mip 0 (which undersamples the source face and produces
    // bright/dark speckle at higher roughness mips).
    prefilterShader.bindUniform(sourceResolution, "sourceResolution");

    for (GLuint mip = 0; mip < maxMipLevels; ++mip) {
        unsigned int mipWidth  = 128u >> mip;
        unsigned int mipHeight = 128u >> mip;

        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        // Re-attach the RBO to the FBO at each mip level. glRenderbufferStorage
        // resizes the RBO but does not update the FBO attachment — the FBO retains
        // its previous depth attachment dimensions, making it incomplete when the
        // mip size changes.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader.bindUniform(roughness, "roughness");

        for (unsigned int i = 0; i < 6; ++i) {
            prefilterShader.bindUniform(captureViews[i], "view");
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, maps.prefilterMap, mip);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "ERROR: Prefilter FBO incomplete on face " << i << " mip " << mip << std::endl;
            }
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Re-bind the source cubemap on every face/mip iteration
            // for the same reason as in the irradiance pass above.
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, sourceCubemap);

            s_cubeVAO->bind();
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
    }

    // ========== BRDF LUT PASS ==========
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);

    glGenTextures(1, &maps.brdfLUT);
    glBindTexture(GL_TEXTURE_2D, maps.brdfLUT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 256, 256, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, maps.brdfLUT, 0);
    glViewport(0, 0, 256, 256);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);

    // ========== CLEANUP ==========
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);

    // Restore caller GL state (viewport would otherwise be left at 256x256,
    // shrinking the next frame's scene render into a corner).
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
    if (prevCull)    glEnable(GL_CULL_FACE);
    if (prevBlend)   glEnable(GL_BLEND);
    if (prevScissor) glEnable(GL_SCISSOR_TEST);

    return maps;
}

}
