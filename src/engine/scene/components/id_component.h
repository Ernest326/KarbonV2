#pragma once
#include <cstdint>
#include <string>

namespace Karbon {

using UUID = uint64_t;

struct IDComponent {
    UUID id = 0;
};

struct TagComponent {
    std::string tag = "Entity";
};

}