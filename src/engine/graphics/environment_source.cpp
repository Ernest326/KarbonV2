#include "graphics/environment_source.h"

namespace Karbon {

EnvironmentSource::~EnvironmentSource() {
    releaseIBLMaps(m_ibl);
}

void EnvironmentSource::generateIBL() {
    if (m_hasIBL) return;
    IBLMaps maps = bakeIBL(getSkyboxCubemap(), getSourceResolution());
    if (maps.irradianceMap == 0) return; // bake failed (no source cubemap)
    m_ibl = maps;
    m_hasIBL = true;
}

}
