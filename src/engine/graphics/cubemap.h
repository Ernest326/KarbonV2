#pragma once
#include "graphics/buffers/buffers.h"
#include "graphics/environment_source.h"
#include <string>
#include <vector>

namespace Karbon {

class Cubemap : public EnvironmentSource {

public:
    Cubemap(const std::vector<std::string> &faces);
    ~Cubemap() override;

    void bind(unsigned int slot = 0) const override;
    void unbind() const override;
    unsigned int getID() const { return m_id; }
    unsigned int getSkyboxCubemap() const override { return m_id; }

protected:
    float getSourceResolution() const override { return m_faceSize; }

private:
    unsigned int m_id = 0;
    float m_faceSize = 512.0f;
    void createVAO();
    static VAO* m_cubemapVAO;
    static IBO* m_cubemapIBO;
};

}
