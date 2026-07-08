#pragma once
#include "graphics/buffers/buffers.h"
#include "graphics/environment_source.h"
#include "graphics/primitives/cube.h"
#include <string>

namespace Karbon {

class EnvironmentMap : public EnvironmentSource {

public:
    explicit EnvironmentMap(const std::string& hdrPath);
    ~EnvironmentMap() override;

    void bind(unsigned int slot = 0) const override;
    void unbind() const override;

    unsigned int getID() const { return m_cubemap; }
    unsigned int getCubemap() const { return m_cubemap; }
    unsigned int getSkyboxCubemap() const override { return m_cubemap; }

protected:
    // The equirect->cubemap conversion always renders 512x512 faces
    float getSourceResolution() const override { return 512.0f; }

private:
    unsigned int m_equirectTexture = 0; // temporary, deleted after conversion
    unsigned int m_cubemap = 0;

    void convertToCubemap();
    void createVAO();
    static VAO* m_envVAO;
    static IBO* m_envIBO;
    static Cube* m_envCube;
};

}
