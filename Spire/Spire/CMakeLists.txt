include(../CMake/Utils/Assets.cmake)

add_library(Spire STATIC
        Source/Engine/Window/Window.cpp
        Source/Engine/Window/Window.h
        Source/Engine/Core/Engine.cpp
        Source/Engine/Core/Engine.h
        Source/Engine/Core/Application.h
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
        Source/Engine/Core/Application.cpp
        ../Game/Assets/Shaders/ShaderInfo.h
        Source/Engine/Utils/Log.h
)

target_include_directories(Spire PUBLIC "Source")
target_include_directories(Spire PUBLIC "Source/Engine")

function(add_lib_subdirectory lib_name)
    add_subdirectory(
            ${LIBS_DIRECTORY}/${lib_name}
            ${CMAKE_CURRENT_BINARY_DIR}/${lib_name}
    )
endfunction()

add_compile_options(/WX- /wd4392)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(BUILD_MODE_DIR "${LIBS_DIRECTORY}/build-release")
    set(DEBUG_SUFFIX "")
else()
    set(BUILD_MODE_DIR "${LIBS_DIRECTORY}/build-debug")
    set(DEBUG_SUFFIX "d")
endif()

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(Spire PUBLIC Vulkan::Vulkan)

# Assimp
target_include_directories(Spire PRIVATE ${LIBS_DIRECTORY}/assimp/include)
target_include_directories(Spire PRIVATE ${BUILD_MODE_DIR}/assimp/include)
target_link_libraries(Spire PRIVATE "${BUILD_MODE_DIR}/assimp-vc143-mt${DEBUG_SUFFIX}.lib")

# GLFW
target_include_directories(Spire PRIVATE ${LIBS_DIRECTORY}/glfw/include)
target_link_libraries(Spire PRIVATE ${BUILD_MODE_DIR}/glfw3.lib)

# GLM
target_include_directories(Spire PRIVATE ${LIBS_DIRECTORY}/glm/include)
target_link_libraries(Spire PRIVATE ${BUILD_MODE_DIR}/glm.lib)

# GLSLang
target_include_directories(Spire PRIVATE ${LIBS_DIRECTORY}/glslang)
target_link_libraries(Spire PRIVATE
        ${BUILD_MODE_DIR}/glslang${DEBUG_SUFFIX}.lib
        ${BUILD_MODE_DIR}/glslang-default-resource-limits${DEBUG_SUFFIX}.lib
        ${BUILD_MODE_DIR}/SPIRV${DEBUG_SUFFIX}.lib
        ${BUILD_MODE_DIR}/SPIRV-Tools${DEBUG_SUFFIX}.lib
        ${BUILD_MODE_DIR}/SPIRV-Tools-diff${DEBUG_SUFFIX}.lib
        ${BUILD_MODE_DIR}/SPIRV-Tools-link${DEBUG_SUFFIX}.lib
        ${BUILD_MODE_DIR}/SPIRV-Tools-lint${DEBUG_SUFFIX}.lib
        ${BUILD_MODE_DIR}/SPIRV-Tools-opt${DEBUG_SUFFIX}.lib
        ${BUILD_MODE_DIR}/SPIRV-Tools-shared${DEBUG_SUFFIX}.lib
)

# ImGui
target_include_directories(Spire PUBLIC ${LIBS_DIRECTORY}/imgui/imgui/)
target_link_libraries(Spire PUBLIC ${BUILD_MODE_DIR}/imgui.lib)

# Vulkan Memory Allocator
target_include_directories(Spire PUBLIC
        ${LIBS_DIRECTORY}/vulkan-memory-allocator/include
)

# stb
target_include_directories(Spire PRIVATE
        ${LIBS_DIRECTORY}/stb/stb
)

# Copy the dlls
file(GLOB LIB_FILES "${BUILD_MODE_DIR}/*.*")

add_custom_command(TARGET Spire POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:Spire>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LIB_FILES}
        $<TARGET_FILE_DIR:Spire>
)