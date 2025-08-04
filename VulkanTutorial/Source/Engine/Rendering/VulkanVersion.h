#pragma once

#include <glm/glm.hpp>

struct VulkanVersion
{
    glm::u32 RawVersion = 0;
    glm::u32 Variant = 0;
    glm::u32 Major = 0;
    glm::u32 Minor = 0;
    glm::u32 Patch = 0;
};
