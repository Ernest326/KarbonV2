#pragma once
#include "../graphics/buffers/buffers.h"
#include <string>
#include <vector>

namespace Karbon {

class Cubemap {

public:
    Cubemap(const std::vector<std::string> &faces);
    ~Cubemap();
    
    void bind(unsigned int slot = 0) const;
    void unbind() const;
    unsigned int getID() const { return m_id; }

    void generateIBL();
    unsigned int getIrradianceMap() const { return m_irradianceMap; }
    unsigned int getPrefilterMap() const { return m_prefilterMap; }
    unsigned int getBRDFLUT() const { return m_brdfLUT; }
    bool hasIBL() const { return m_hasIBL; }

private:
    unsigned int m_id;
    void createVAO();
    static VAO* m_cubemapVAO;
    static IBO* m_cubemapIBO;

    GLuint m_irradianceMap = 0;
    GLuint m_prefilterMap = 0;
    GLuint m_brdfLUT = 0;
    bool m_hasIBL = false;
};

}