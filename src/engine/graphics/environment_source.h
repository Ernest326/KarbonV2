#pragma once
#include "graphics/ibl_baker.h"

namespace Karbon {

// Common interface for skybox/IBL environment sources (LDR face cubemap,
// HDR equirectangular map). Owns the baked IBL maps; concrete classes
// provide the source cubemap texture and its skybox VAO binding.
class EnvironmentSource {
public:
    EnvironmentSource() = default;
    virtual ~EnvironmentSource();

    // Owns GL texture handles
    EnvironmentSource(const EnvironmentSource&) = delete;
    EnvironmentSource& operator=(const EnvironmentSource&) = delete;

    // Bind the skybox cubemap (and its cube VAO) for rendering
    virtual void bind(unsigned int slot = 0) const = 0;
    virtual void unbind() const = 0;

    virtual unsigned int getSkyboxCubemap() const = 0;

    void generateIBL();
    bool hasIBL() const { return m_hasIBL; }
    unsigned int getIrradianceMap() const { return m_ibl.irradianceMap; }
    unsigned int getPrefilterMap() const { return m_ibl.prefilterMap; }
    unsigned int getBRDFLUT() const { return m_ibl.brdfLUT; }

protected:
    // Pixel size of one face of the source cubemap (drives prefilter mip selection)
    virtual float getSourceResolution() const = 0;

private:
    IBLMaps m_ibl;
    bool m_hasIBL = false;
};

}
