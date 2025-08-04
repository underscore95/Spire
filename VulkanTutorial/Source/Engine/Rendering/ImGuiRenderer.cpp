#include "ImGuiRenderer.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <spdlog/spdlog.h>

#include "Renderer.h"
#include "RenderingCommandManager.h"
#include "RenderingDeviceManager.h"
#include "RenderingManager.h"
#include "RenderingSync.h"
#include "Swapchain.h"
#include "VulkanQueue.h"
#include "Engine/Window/Window.h"

ImGuiRenderer::ImGuiRenderer(
    RenderingManager& renderingManager,
    const Window& window)
    : m_renderingManager(renderingManager),
      m_window(window)
{
    CreateDescriptorPool();
    InitImGUI();

    spdlog::info("Initialized ImGui renderer");
}

ImGuiRenderer::~ImGuiRenderer()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(m_renderingManager.GetDevice(), m_descriptorPool, nullptr);
    m_renderingManager.GetCommandManager().FreeCommandBuffers(m_commandBuffers.size(), m_commandBuffers.data());

    spdlog::info("Shut down ImGui renderer");
}

VkCommandBuffer ImGuiRenderer::PrepareCommandBuffer(glm::u32 imageIndex) const
{
    m_renderingManager.GetCommandManager().BeginCommandBuffer(m_commandBuffers[imageIndex],
                                                              VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    m_renderingManager.GetRenderer().BeginRendering(m_commandBuffers[imageIndex], imageIndex, nullptr, nullptr);

    ImDrawData* pDrawData = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(pDrawData, m_commandBuffers[imageIndex]);

    vkCmdEndRendering(m_commandBuffers[imageIndex]);

    m_renderingManager.GetRenderingSync().ImageMemoryBarrier(m_commandBuffers[imageIndex],
                                                             m_renderingManager.GetSwapchain().GetImage(imageIndex),
                                                             m_renderingManager.GetSwapchain().GetSurfaceFormat().
                                                             format,
                                                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                             VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    vkEndCommandBuffer(m_commandBuffers[imageIndex]);

    return m_commandBuffers[imageIndex];
}

void ImGuiRenderer::CreateDescriptorPool()
{
    VkDescriptorPoolSize PoolSizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    };

    VkDescriptorPoolCreateInfo poolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000 * IM_ARRAYSIZE(PoolSizes),
        .poolSizeCount = static_cast<glm::u32>(IM_ARRAYSIZE(PoolSizes)),
        .pPoolSizes = PoolSizes
    };

    VkResult res = vkCreateDescriptorPool(m_renderingManager.GetDevice(), &poolCreateInfo, nullptr, &m_descriptorPool);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to create descriptor pool for imgui");
    }
}

void ImGuiRenderer::InitImGUI()
{
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    io.DisplaySize.x = m_window.GetDimensions().x;
    io.DisplaySize.y = m_window.GetDimensions().y;

    ImGui::StyleColorsDark();

    bool installGLFWCallbacks = true;
    ImGui_ImplGlfw_InitForVulkan(m_window.GLFWWindow(), installGLFWCallbacks);

    VkFormat colorFormat = m_renderingManager.GetSwapchain().GetSurfaceFormat().format;

    ImGui_ImplVulkan_InitInfo initInfo = {
        .Instance = m_renderingManager.GetInstance(),
        .PhysicalDevice = m_renderingManager.GetPhysicalDevice().PhysicalDeviceHandle,
        .Device = m_renderingManager.GetDevice(),
        .QueueFamily = m_renderingManager.GetQueueFamily(),
        .Queue = m_renderingManager.GetQueue().GetQueueHandle(),
        .DescriptorPool = m_descriptorPool,
        .RenderPass = nullptr,
        .MinImageCount = m_renderingManager.GetPhysicalDevice().SurfaceCapabilities.minImageCount,
        .ImageCount = m_renderingManager.GetSwapchain().GetNumImages(),
        .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        .PipelineCache = nullptr,
        .Subpass = 0,
        .UseDynamicRendering = true,
        .PipelineRenderingCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
            .pNext = nullptr,
            .viewMask = 0,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &colorFormat,
            .depthAttachmentFormat = m_renderingManager.GetPhysicalDevice().DepthFormat,
            .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
        },
        .Allocator = nullptr,
        .CheckVkResultFn = CheckVkResult
    };

    ImGui_ImplVulkan_Init(&initInfo);

    m_commandBuffers.resize(m_renderingManager.GetSwapchain().GetNumImages());
    m_renderingManager.GetCommandManager().CreateCommandBuffers(m_commandBuffers.size(), m_commandBuffers.data());
}

void ImGuiRenderer::CheckVkResult(VkResult res)
{
    if (res == VK_SUCCESS) return;

    spdlog::error("ImGui Vulkan function finished with code {}", static_cast<int>(res));
}
