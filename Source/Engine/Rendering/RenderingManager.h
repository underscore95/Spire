#pragma once

#include <string>
#include <vulkan/vulkan.h>"

class RenderingManager {
public:
    explicit RenderingManager(const std::string &applicationName);

    ~RenderingManager();

public:
    bool IsValid() const;

private:
    void CreateInstance(const std::string &applicationName);

private:
    VkInstance m_instance = nullptr;
};
