#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#include "Engine/EngineIncludes.h"

class GameApplication : public Application {
public:
    void Start(Engine &engine) override;

    ~GameApplication() override;

    void Update() override;

    void Render() override;

    [[nodiscard]] bool ShouldClose() const override;

    [[nodiscard]] std::string GetApplicationName() const override;

private:
    Engine *m_engine = nullptr;
    std::vector<VkCommandBuffer> m_commandBuffers;
};
