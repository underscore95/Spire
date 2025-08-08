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
        Source/Engine/Rendering/VulkanUtils.h
        Source/Engine/Rendering/RenderingDeviceManager.cpp
        Source/Engine/Rendering/RenderingDeviceManager.h
        Source/Engine/Rendering/RenderingCommandManager.cpp
        Source/Engine/Rendering/RenderingCommandManager.h
        Source/Engine/Rendering/VulkanQueue.cpp
        Source/Engine/Rendering/VulkanQueue.h
        Source/Engine/Rendering/ShaderCompiler.cpp
        Source/Engine/Rendering/ShaderCompiler.h
        Source/Engine/Rendering/GraphicsPipeline.cpp
        Source/Engine/Rendering/GraphicsPipeline.h
        Source/Engine/Rendering/BufferManager.cpp
        Source/Engine/Rendering/BufferManager.h
        Source/Engine/Utils/Timer.cpp
        Source/Engine/Utils/Timer.h
        Source/Engine/Rendering/PipelineDescriptorSetsManager.cpp
        Source/Engine/Rendering/PipelineDescriptorSetsManager.h
        Source/Engine/Utils/Camera/Camera.cpp
        Source/Engine/Utils/Camera/Camera.h
        Source/Engine/Rendering/TextureManager.cpp
        Source/Engine/Rendering/TextureManager.h
        Source/Engine/Resources/ImageLoader.cpp
        Source/Engine/Resources/ImageLoader.h
        Source/Engine/Utils/FileIO.cpp
        Source/Engine/Utils/FileIO.h
        Source/Engine/Rendering/Swapchain.cpp
        Source/Engine/Rendering/Swapchain.h
        Source/Engine/Rendering/VulkanDebugCallback.cpp
        Source/Engine/Rendering/VulkanDebugCallback.h
        Source/Engine/Rendering/LogicalDevice.cpp
        Source/Engine/Rendering/LogicalDevice.h
        Source/Engine/Rendering/VulkanVersion.h
        Source/Engine/Rendering/RenderingSync.cpp
        Source/Engine/Rendering/RenderingSync.h
        Source/Engine/Rendering/ImGuiRenderer.cpp
        Source/Engine/Rendering/ImGuiRenderer.h
        Source/Engine/Rendering/Renderer.cpp
        Source/Engine/Rendering/Renderer.h
        Source/Engine/Rendering/VulkanAllocator.cpp
        Source/Engine/Rendering/VulkanAllocator.h
        Source/Engine/Rendering/VmaUsage.cpp
        Source/Engine/Rendering/VulkanBuffer.h
        Source/Engine/Rendering/VulkanImage.h
        Source/Engine/Resources/Mesh.h
        Source/Engine/Rendering/SceneModels.cpp
        Source/Engine/Rendering/SceneModels.h
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