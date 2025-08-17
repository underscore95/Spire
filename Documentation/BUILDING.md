# Building

Spire is a CMake project using C++23 and libraries are managed using a package manager.

The below instructions assume you use vcpkg as your package manager.

1. Follow these instructions to setup vcpkg on your system and use it with CMake: https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell
2. Download the latest Vulkan SDK: https://www.lunarg.com/vulkan-sdk/
3. Run `vcpkg install vulkan-memory-allocator assimp glslang glfw3 glm spdlog libassert imgui imgui[vulkan-binding] imgui[glfw-binding]`