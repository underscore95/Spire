#pragma once

#include <atomic>
#include <condition_variable>
#include <vulkan/vulkan.h>
#include <string>

class ShaderCompiler
{
public:
    explicit ShaderCompiler(VkDevice device, bool alwaysCompileFromSource = false);
    ~ShaderCompiler();

public:
    // Do not attempt to compile the same file twice async without awaiting between the calls
    void CreateShaderModuleAsync(VkShaderModule* out, const std::string& fileName) ;
    void Await();

    VkShaderModule CreateShaderModule(const std::string& fileName) const;

private:
    VkShaderModule CreateShaderModuleFromBinaryFile(const std::string& fileName) const;
    VkShaderModule CreateShaderModuleFromSource(const std::string& fileName, const std::string& shaderSource) const;

private:
    VkDevice m_device;
    bool m_alwaysCompileFromSource;
    std::atomic_int m_currentTasks = 0;
    std::mutex m_waitForTasksMutex;
    std::condition_variable m_waitForTasksCompleteCv;
};
