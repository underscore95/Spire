cmake_minimum_required(VERSION 3.31)
project(VulkanTutorial)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

add_executable(VulkanTutorial
        Source/Main.cpp
        Source/Engine/Window/window.cpp
        Source/Engine/Window/window.h
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
)

target_include_directories(VulkanTutorial PUBLIC "Source/")

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(VulkanTutorial PRIVATE Vulkan::Vulkan)

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