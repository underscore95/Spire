#pragma once

#include <vulkan/vulkan.h>

class ShaderCompiler {
public:
    explicit ShaderCompiler(VkDevice device);

    VkShaderModule CreateShaderModuleFromBinary(const char *pFilename) const;

  VkShaderModule CreateShaderModuleFromText(const char *pFilename) const;

private:
    VkDevice m_device;
};
