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

private:
    unsigned int m_id;
    void createVAO();
    static VAO* m_cubemapVAO;
    static IBO* m_cubemapIBO;

};

}