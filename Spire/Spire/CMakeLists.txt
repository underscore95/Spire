include(../CMake/Utils/Assets.cmake)

add_library(Spire STATIC
        Source/Window/Window.cpp
        Source/Window/Window.h
        Source/Core/Engine.cpp
        Source/Core/Engine.h
        Source/Core/Application.h
        Source/EngineIncludes.h
        Source/Rendering/RenderingManager.cpp
        Source/Rendering/RenderingManager.h
        Source/Rendering/Core/VulkanUtils.h
        Source/Rendering/Core/RenderingDeviceManager.cpp
        Source/Rendering/Core/RenderingDeviceManager.h
        Source/Rendering/Core/RenderingCommandManager.cpp
        Source/Rendering/Core/RenderingCommandManager.h
        Source/Rendering/Core/VulkanQueue.cpp
        Source/Rendering/Core/VulkanQueue.h
        Source/Rendering/Core/ShaderCompiler.cpp
        Source/Rendering/Core/ShaderCompiler.h
        Source/Rendering/Core/Pipelines/GraphicsPipeline.cpp
        Source/Rendering/Core/Pipelines/GraphicsPipeline.h
        Source/Rendering/Memory/BufferManager.cpp
        Source/Rendering/Memory/BufferManager.h
        Source/Utils/Timer.cpp
        Source/Utils/Timer.h
        Source/Utils/Camera/Camera.cpp
        Source/Utils/Camera/Camera.h
        Source/Rendering/Memory/ImageManager.cpp
        Source/Rendering/Memory/ImageManager.h
        Source/Resources/ImageLoader.cpp
        Source/Resources/ImageLoader.h
        Source/Utils/FileIO.cpp
        Source/Utils/FileIO.h
        Source/Rendering/Core/Swapchain.cpp
        Source/Rendering/Core/Swapchain.h
        Source/Rendering/Core/VulkanDebugCallback.cpp
        Source/Rendering/Core/VulkanDebugCallback.h
        Source/Rendering/Core/LogicalDevice.cpp
        Source/Rendering/Core/LogicalDevice.h
        Source/Rendering/Core/VulkanVersion.h
        Source/Rendering/Core/RenderingSync.cpp
        Source/Rendering/Core/RenderingSync.h
        Source/Rendering/Renderers/ImGuiRenderer.cpp
        Source/Rendering/Renderers/ImGuiRenderer.h
        Source/Rendering/Renderers/Renderer.cpp
        Source/Rendering/Renderers/Renderer.h
        Source/Rendering/Memory/VulkanAllocator.cpp
        Source/Rendering/Memory/VulkanAllocator.h
        Source/Rendering/Memory/VmaUsage.cpp
        Source/Rendering/Memory/VulkanBuffer.h
        Source/Rendering/Memory/VulkanImage.h
        Source/Resources/Mesh.h
        Source/Rendering/Scene/SceneModels.cpp
        Source/Rendering/Scene/SceneModels.h
        Source/Rendering/Scene/PushConstants.h
        Source/Resources/ModelLoader.cpp
        Source/Resources/ModelLoader.h
        Source/Resources/Model.h
        Source/Rendering/Scene/SceneImages.cpp
        Source/Rendering/Scene/SceneImages.h
        Source/Rendering/Descriptors/Descriptor.h
        Source/Rendering/Descriptors/DescriptorPool.cpp
        Source/Rendering/Descriptors/DescriptorPool.h
        Source/Rendering/Descriptors/DescriptorSet.h
        Source/Rendering/Descriptors/DescriptorSetsUpdater.cpp
        Source/Rendering/Descriptors/DescriptorSetsUpdater.h
        Source/Rendering/Descriptors/DescriptorManager.cpp
        Source/Rendering/Descriptors/DescriptorManager.h
        Source/Rendering/Descriptors/DescriptorSetLayoutList.cpp
        Source/Rendering/Descriptors/DescriptorSetLayoutList.h
        Source/Rendering/Descriptors/DescriptorCreator.cpp
        Source/Rendering/Descriptors/DescriptorCreator.h
        Source/Rendering/Core/Pipelines/Pipeline.cpp
        Source/Rendering/Core/Pipelines/Pipeline.h
        Source/Rendering/Core/Pipelines/ComputePipeline.cpp
        Source/Rendering/Core/Pipelines/ComputePipeline.h
        Source/Rendering/Memory/PerImageBuffer.cpp
        Source/Rendering/Memory/PerImageBuffer.h
        Source/Utils/MacroDisableCopy.h
        Source/Core/Application.cpp
        Source/Utils/Log.h
        Source/Utils/Version.cpp
        Source/Utils/Version.h
        Source/pch.h
        Source/Utils/Delegates/Delegate.h
        Source/Utils/Delegates/DelegateSubscriber.h
        Source/Utils/Hashing.h
)

target_precompile_headers(Spire PUBLIC "Source/pch.h")

target_include_directories(Spire PUBLIC "Source")

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
file(GLOB LIB_FILES CONFIGURE_DEPENDS "${BUILD_MODE_DIR}/*.dll")

add_custom_command(TARGET Spire POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:Spire>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LIB_FILES}
        ${CMAKE_BINARY_DIR}/bin
)