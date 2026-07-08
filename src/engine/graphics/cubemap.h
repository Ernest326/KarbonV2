#pragma once
#include "graphics/buffers/buffers.h"
#include <string>
#include <vector>

namespace Karbon {

class Cubemap {

public:
    Cubemap(const std::vector<std::string> &faces);
    ~Cubemap();

    // Owns GL texture handles: non-copyable, movable
    Cubemap(const Cubemap&) = delete;
    Cubemap& operator=(const Cubemap&) = delete;
    Cubemap(Cubemap&& other) noexcept
        : m_id(other.m_id), m_irradianceMap(other.m_irradianceMap),
          m_prefilterMap(other.m_prefilterMap), m_brdfLUT(other.m_brdfLUT),
          m_hasIBL(other.m_hasIBL) {
        other.m_id = 0;
        other.m_irradianceMap = 0;
        other.m_prefilterMap = 0;
        other.m_brdfLUT = 0;
        other.m_hasIBL = false;
    }
    Cubemap& operator=(Cubemap&& other) noexcept {
        if (this != &other) {
            release();
            m_id = other.m_id;
            m_irradianceMap = other.m_irradianceMap;
            m_prefilterMap = other.m_prefilterMap;
            m_brdfLUT = other.m_brdfLUT;
            m_hasIBL = other.m_hasIBL;
            other.m_id = 0;
            other.m_irradianceMap = 0;
            other.m_prefilterMap = 0;
            other.m_brdfLUT = 0;
            other.m_hasIBL = false;
        }
        return *this;
    }


    void bind(unsigned int slot = 0) const;
    void unbind() const;
    unsigned int getID() const { return m_id; }

    void generateIBL();
    unsigned int getIrradianceMap() const { return m_irradianceMap; }
    unsigned int getPrefilterMap() const { return m_prefilterMap; }
    unsigned int getBRDFLUT() const { return m_brdfLUT; }
    bool hasIBL() const { return m_hasIBL; }

private:
    void release();

    unsigned int m_id = 0;
    void createVAO();
    static VAO* m_cubemapVAO;
    static IBO* m_cubemapIBO;

    GLuint m_irradianceMap = 0;
    GLuint m_prefilterMap = 0;
    GLuint m_brdfLUT = 0;
    bool m_hasIBL = false;
};

}