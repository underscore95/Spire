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
)

target_include_directories(VulkanTutorial PUBLIC "Source/")

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(VulkanTutorial PRIVATE ${VULKAN_LIB_LIST})
target_include_directories(VulkanTutorial PUBLIC ${VULKAN_INCLUDE_DIRS})

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