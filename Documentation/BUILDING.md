# Building

Spire is a CMake project using C++23. It has only been tested on Windows 11 using a NVIDIA GPU.

The project depends on assimp, glfw, glm, glslang, imgui, spdlog, stb, vulkan, vulkan-memory-allocator, BS thread pool, GTest.

## Steps to build
1. Libraries are included as git submodules, ensure --recurse-submodules flag is used when cloning the repository.
2. Install Python 3, CMake 3.31+, Visual Studio 17 2022, and Vulkan SDK. Ensure Visual Studio are in your environment path. Ensure CMake is in your environment path or located at `C:\Program Files\CMake\bin`.
3. Run build.py in Spire/Libs, this builds all the libraries in debug and release mode and will take about 5-15 minutes to complete.
3. Reload CMake project.
4. Build and launch target Game, this will also run all unit tests.

# Worlds

Worlds should be in a Worlds directory which should be in the same directory as the executable, e.g. Worlds/Test8

In release mode, the world to load can be set by changing PROFILE_WORLD_NAME in Profiling.h in the Game project.

# Minecraft Assets

The default GameApplication code relies on Minecraft assets, see [MinecraftRegionConverter README](https://github.com/underscore95/Spire/tree/main/Tooling/MinecraftRegionConverter) for more information.
