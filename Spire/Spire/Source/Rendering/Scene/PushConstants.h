#pragma once

#include "pch.h"

namespace Spire
{
    // You can make your own push constants but keep the engine ones at the beginning of the struct (make your own struct with this as a field)
    // alignment: https://registry.khronos.org/OpenGL/specs/gl/glspec45.core.pdf#page=159
    struct alignas(16) PushConstants {
        glm::u32 ImageIndex;
    };
}