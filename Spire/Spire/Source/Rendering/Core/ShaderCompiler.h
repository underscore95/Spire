#pragma once

#include "pch.h"

namespace Spire
{
    class ShaderCompiler
    {
    public:
        struct Options {
            bool Optimise = true;
            bool OptimiseSize = true;
        };
    public:
        explicit ShaderCompiler(VkDevice device, bool alwaysCompileFromSource = false);
        ~ShaderCompiler();

    public:
        // Do not attempt to compile the same file twice async without awaiting between the calls
        void CreateShaderModuleAsync(VkShaderModule* out, const std::string& fileName, Options options = {}) ;
        void Await();

        VkShaderModule CreateShaderModule(const std::string& fileName, Options options = {}) const;

    private:
        VkShaderModule CreateShaderModuleFromBinaryFile(const std::string& fileName) const;
        VkShaderModule CreateShaderModuleFromSource(const std::string &fileName, const std::string &shaderSource, Options options) const;

    private:
        VkDevice m_device;
        bool m_alwaysCompileFromSource;
        std::atomic_int m_currentTasks = 0;
        std::mutex m_waitForTasksMutex;
        std::condition_variable m_waitForTasksCompleteCv;
    };
}