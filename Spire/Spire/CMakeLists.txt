include(../CMake/Utils/Assets.cmake)

add_executable(Spire
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
        Source/Engine/Rendering/Core/Pipelines/GraphicsPipeline.cpp
        Source/Engine/Rendering/Core/Pipelines/GraphicsPipeline.h
        Source/Engine/Rendering/Memory/BufferManager.cpp
        Source/Engine/Rendering/Memory/BufferManager.h
        Source/Engine/Utils/Timer.cpp
        Source/Engine/Utils/Timer.h
        Source/Engine/Utils/Camera/Camera.cpp
        Source/Engine/Utils/Camera/Camera.h
        Source/Engine/Rendering/Memory/ImageManager.cpp
        Source/Engine/Rendering/Memory/ImageManager.h
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
        Source/Engine/Rendering/Scene/SceneImages.cpp
        Source/Engine/Rendering/Scene/SceneImages.h
        Source/Engine/Rendering/Descriptors/Descriptor.h
        Source/Engine/Rendering/Descriptors/DescriptorPool.cpp
        Source/Engine/Rendering/Descriptors/DescriptorPool.h
        Source/Engine/Rendering/Descriptors/DescriptorSet.h
        Source/Engine/Rendering/Descriptors/DescriptorSetsUpdater.cpp
        Source/Engine/Rendering/Descriptors/DescriptorSetsUpdater.h
        Source/Engine/Rendering/Descriptors/DescriptorManager.cpp
        Source/Engine/Rendering/Descriptors/DescriptorManager.h
        Source/Engine/Rendering/Descriptors/DescriptorSetLayoutList.cpp
        Source/Engine/Rendering/Descriptors/DescriptorSetLayoutList.h
        Source/Engine/Rendering/Descriptors/DescriptorCreator.cpp
        Source/Engine/Rendering/Descriptors/DescriptorCreator.h
        Source/Engine/Rendering/Core/Pipelines/Pipeline.cpp
        Source/Engine/Rendering/Core/Pipelines/Pipeline.h
        Source/Engine/Rendering/Core/Pipelines/ComputePipeline.cpp
        Source/Engine/Rendering/Core/Pipelines/ComputePipeline.h
        Source/Engine/Rendering/Memory/PerImageBuffer.cpp
        Source/Engine/Rendering/Memory/PerImageBuffer.h
        Source/Engine/Utils/MacroDisableCopy.h
        Source/Game/GameCamera.cpp
        Source/Game/GameCamera.h
        Source/Game/RenderInfo.h
        Source/Engine/Core/Application.cpp
        Assets/Shaders/ShaderInfo.h
        Source/Game/ObjectRenderer.cpp
        Source/Game/ObjectRenderer.h
)

target_include_directories(Spire PUBLIC "Source")

function(add_lib_subdirectory lib_name)
    add_subdirectory(
            ${LIBS_DIRECTORY}/${lib_name}
            ${CMAKE_CURRENT_BINARY_DIR}/${lib_name}
    )
endfunction()

add_compile_options(/WX- /wd4392)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(Spire PRIVATE Vulkan::Vulkan)

# Vulkan Memory Allocator
add_lib_subdirectory(vulkan-memory-allocator)
target_link_libraries(Spire PRIVATE GPUOpen::VulkanMemoryAllocator)

# assimp
add_lib_subdirectory(assimp)
target_link_libraries(Spire PRIVATE assimp::assimp)

# glfw3
add_lib_subdirectory(glfw)
target_link_libraries(Spire PRIVATE glfw)

# glm
add_lib_subdirectory(glm)
target_link_libraries(Spire PRIVATE glm::glm-header-only)

# spdlog
add_lib_subdirectory(spdlog)
target_link_libraries(Spire PRIVATE spdlog::spdlog)

# imgui
add_lib_subdirectory(imgui)
target_link_libraries(Spire PRIVATE imgui)

# glslang
add_lib_subdirectory(glslang)
target_link_libraries(Spire PRIVATE glslang::glslang)

# stb
add_lib_subdirectory(stb)
include_stb_to_target(Spire)

# Assets directory
setup_assets_directory(Spire Assets)