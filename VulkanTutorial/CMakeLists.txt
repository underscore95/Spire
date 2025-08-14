include(../CMake/Utils/Assets.cmake)

add_executable(VulkanTutorial
        Source/Main.cpp
        Source/Engine/Window/Window.cpp
        Source/Engine/Window/Window.h
        Source/Engine/Core/Engine.cpp
        Source/Engine/Core/Engine.h
        Source/Engine/Core/Application.h
        Source/Game/GameApplication.cpp
        Source/Game/GameApplication.h
        Source/Engine/EngineIncludes.h
        Source/Engine/Rendering/RenderingManager.cpp
        Source/Engine/Rendering/RenderingManager.h
        Source/Engine/Rendering/Core/VulkanUtils.h
        Source/Engine/Rendering/Core/RenderingDeviceManager.cpp
        Source/Engine/Rendering/Core/RenderingDeviceManager.h
        Source/Engine/Rendering/Core/RenderingCommandManager.cpp
        Source/Engine/Rendering/Core/RenderingCommandManager.h
        Source/Engine/Rendering/Core/VulkanQueue.cpp
        Source/Engine/Rendering/Core/VulkanQueue.h
        Source/Engine/Rendering/Core/ShaderCompiler.cpp
        Source/Engine/Rendering/Core/ShaderCompiler.h
        Source/Engine/Rendering/Core/GraphicsPipeline.cpp
        Source/Engine/Rendering/Core/GraphicsPipeline.h
        Source/Engine/Rendering/Memory/BufferManager.cpp
        Source/Engine/Rendering/Memory/BufferManager.h
        Source/Engine/Utils/Timer.cpp
        Source/Engine/Utils/Timer.h
        Source/Engine/Utils/Camera/Camera.cpp
        Source/Engine/Utils/Camera/Camera.h
        Source/Engine/Rendering/Memory/TextureManager.cpp
        Source/Engine/Rendering/Memory/TextureManager.h
        Source/Engine/Resources/ImageLoader.cpp
        Source/Engine/Resources/ImageLoader.h
        Source/Engine/Utils/FileIO.cpp
        Source/Engine/Utils/FileIO.h
        Source/Engine/Rendering/Core/Swapchain.cpp
        Source/Engine/Rendering/Core/Swapchain.h
        Source/Engine/Rendering/Core/VulkanDebugCallback.cpp
        Source/Engine/Rendering/Core/VulkanDebugCallback.h
        Source/Engine/Rendering/Core/LogicalDevice.cpp
        Source/Engine/Rendering/Core/LogicalDevice.h
        Source/Engine/Rendering/Core/VulkanVersion.h
        Source/Engine/Rendering/Core/RenderingSync.cpp
        Source/Engine/Rendering/Core/RenderingSync.h
        Source/Engine/Rendering/Renderers/ImGuiRenderer.cpp
        Source/Engine/Rendering/Renderers/ImGuiRenderer.h
        Source/Engine/Rendering/Renderers/Renderer.cpp
        Source/Engine/Rendering/Renderers/Renderer.h
        Source/Engine/Rendering/Memory/VulkanAllocator.cpp
        Source/Engine/Rendering/Memory/VulkanAllocator.h
        Source/Engine/Rendering/Memory/VmaUsage.cpp
        Source/Engine/Rendering/Memory/VulkanBuffer.h
        Source/Engine/Rendering/Memory/VulkanImage.h
        Source/Engine/Resources/Mesh.h
        Source/Engine/Rendering/Scene/SceneModels.cpp
        Source/Engine/Rendering/Scene/SceneModels.h
        Source/Engine/Rendering/Scene/PushConstants.h
        Source/Engine/Resources/ModelLoader.cpp
        Source/Engine/Resources/ModelLoader.h
        Source/Engine/Resources/Model.h
        Source/Engine/Rendering/Scene/SceneTextures.cpp
        Source/Engine/Rendering/Scene/SceneTextures.h
        Source/Engine/Rendering/Descriptors/Descriptor.h
        Source/Engine/Rendering/Descriptors/DescriptorPool.cpp
        Source/Engine/Rendering/Descriptors/DescriptorPool.h
        Source/Engine/Rendering/Descriptors/DescriptorSet.h
        Source/Engine/Rendering/Descriptors/DescriptorSetsUpdater.cpp
        Source/Engine/Rendering/Descriptors/DescriptorSetsUpdater.h
        Source/Engine/Rendering/Descriptors/DescriptorManager.cpp
        Source/Engine/Rendering/Descriptors/DescriptorManager.h
        Source/Engine/Rendering/Descriptors/DescriptorSetLayout.cpp
        Source/Engine/Rendering/Descriptors/DescriptorSetLayout.h
)

target_include_directories(VulkanTutorial PUBLIC "Source")

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(VulkanTutorial PRIVATE Vulkan::Vulkan)

# Vulkan Memory Allocator
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(VulkanTutorial PRIVATE GPUOpen::VulkanMemoryAllocator)

# assimp
find_package(assimp CONFIG REQUIRED)
target_link_libraries(VulkanTutorial PRIVATE assimp::assimp)

# glslang
find_package(glslang REQUIRED)
target_link_libraries(VulkanTutorial PRIVATE glslang::glslang)

# glfw3
find_package(glfw3 REQUIRED)
target_link_libraries(VulkanTutorial PRIVATE glfw)

# glm
find_package(glm REQUIRED)
target_link_libraries(VulkanTutorial PRIVATE glm::glm-header-only)

# spdlog
find_package(spdlog REQUIRED)
target_link_libraries(VulkanTutorial PRIVATE spdlog::spdlog)

# libassert
find_package(libassert REQUIRED)
target_link_libraries(VulkanTutorial PRIVATE libassert::assert)

# imgui
find_package(imgui CONFIG REQUIRED)
target_link_libraries(VulkanTutorial PRIVATE imgui::imgui)

# Assets directory
setup_assets_directory(VulkanTutorial Assets)