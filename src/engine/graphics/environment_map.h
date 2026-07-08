#pragma once
#include "graphics/buffers/buffers.h"
#include "graphics/shader.h" 
#include "graphics/primitives/cube.h"
#include <string>

namespace Karbon {

class EnvironmentMap {

public:
    explicit EnvironmentMap(const std::string& hdrPath);
    ~EnvironmentMap();

    // Owns GL texture handles; held via unique_ptr, so copying/moving is disabled
    EnvironmentMap(const EnvironmentMap&) = delete;
    EnvironmentMap& operator=(const EnvironmentMap&) = delete;
    EnvironmentMap(EnvironmentMap&&) = delete;
    EnvironmentMap& operator=(EnvironmentMap&&) = delete;

    void bind(unsigned int slot = 0) const;
    void unbind() const;

    unsigned int getID() const { return m_cubemap; }
    unsigned int getCubemap() const { return m_cubemap; }

    void generateIBL();
    unsigned int getIrradianceMap() const { return m_irradianceMap; }
    unsigned int getPrefilterMap() const { return m_prefilterMap; }
    unsigned int getBRDFLUT() const { return m_brdfLUT; }
    bool hasIBL() const { return m_hasIBL; }
private:
    unsigned int m_equirectTexture = 0; // temporary, deleted after conversion
    unsigned int m_cubemap = 0;

    void convertToCubemap();
    void createVAO();
    static VAO* m_envVAO;
    static IBO* m_envIBO;

    GLuint m_irradianceMap = 0;
    GLuint m_prefilterMap = 0;
    GLuint m_brdfLUT = 0;
    bool m_hasIBL = false;
    static Cube* m_envCube;
};

}