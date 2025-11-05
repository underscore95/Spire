# Building

Spire is a CMake project using C++23.

The project depends on assimp, glfw, glm, glslang, imgui, spdlog, stb, vulkan, vulkan-memory-allocator.

## Steps to build
1. Run build.py in Spire/Libs, this builds all the libraries in debug and release mode and will take about 5-10 minutes to complete.
2. Reload CMake project
3. Build target Game, this will take 1-2 minutes.