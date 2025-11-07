#pragma once

#include "pch.h"

namespace Spire
{
    struct VulkanVersion
    {
        glm::u32 RawVersion = 0;
        glm::u32 Variant = 0;
        glm::u32 Major = 0;
        glm::u32 Minor = 0;
        glm::u32 Patch = 0;

        bool operator>=(const VulkanVersion & vulkanVersion) const {
            return RawVersion >= vulkanVersion.RawVersion;
        }

        bool operator<(const VulkanVersion & vulkanVersion) const {
            return vulkanVersion >= *this;
        };
    };
}