
add_library(SpireVoxel STATIC
        Source/VoxelRenderer.cpp
        Source/VoxelRenderer.h
        Source/Rendering/GameCamera.cpp
        Source/Rendering/GameCamera.h
        Source/Rendering/RenderInfo.h
        Source/Chunk/Chunk.cpp
        Source/Chunk/Chunk.h
        Source/Chunk/VoxelWorld.cpp
        Source/Chunk/VoxelWorld.h
        Source/Rendering/BufferAllocator.cpp
        Source/Rendering/BufferAllocator.h
        Source/Serialisation/VoxelSerializer.cpp
        Source/Serialisation/VoxelSerializer.h
        Source/Rendering/VoxelWorldRenderer.cpp
        Source/Rendering/VoxelWorldRenderer.h
        Source/Types/VoxelTypeRegistry.cpp
        Source/Types/VoxelTypeRegistry.h
        Source/Types/VoxelType.h
        Source/Types/RegisteredVoxelType.h
        Source/Types/VoxelImageManager.cpp
        Source/Types/VoxelImageManager.h
)

target_include_directories(SpireVoxel PUBLIC "Source/")
target_link_libraries(SpireVoxel PUBLIC Spire)

setup_assets_directory(SpireVoxel Assets)

# Tests
add_subdirectory(Tests)