#pragma once
#include <glad/glad.h>

namespace Karbon {

struct IBLMaps {
    GLuint irradianceMap = 0;
    GLuint prefilterMap = 0;
    GLuint brdfLUT = 0;
};

// Bakes diffuse irradiance, specular prefilter and BRDF-LUT maps from an
// environment cubemap. Generates mipmaps on the source texture (the prefilter
// pass samples source mips from the GGX PDF), so the source's min filter is
// switched to GL_LINEAR_MIPMAP_LINEAR. Caller owns the returned textures.
// sourceResolution is the pixel size of one source cubemap face.
IBLMaps bakeIBL(GLuint sourceCubemap, float sourceResolution);

void releaseIBLMaps(IBLMaps& maps);

}
