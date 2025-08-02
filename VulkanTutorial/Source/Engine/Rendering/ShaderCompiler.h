#pragma once

#include <vulkan/vulkan.h>

#define SHADER_COMPILER_USING_STD 1
#ifdef SHADER_COMPILER_USING_STD
#include <string>
#endif

class ShaderCompiler {
public:
    explicit ShaderCompiler(VkDevice device);

    VkShaderModule CreateShaderModuleFromBinary(const char *pFilename) const;

#ifdef SHADER_COMPILER_USING_STD
    VkShaderModule CreateShaderModuleFromBinary(const std::string& fileName) const;
#endif

  VkShaderModule CreateShaderModuleFromText(const char *pFilename) const;

#ifdef SHADER_COMPILER_USING_STD
  VkShaderModule CreateShaderModuleFromText(const std::string& fileName) const;
#endif

private:
    VkDevice m_device;
};
