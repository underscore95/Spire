
add_library(SpireVoxel STATIC
        "Source/VoxelRenderer.cpp"
        "Source/VoxelRenderer.h"
        Source/Rendering/GameCamera.cpp
        Source/Rendering/GameCamera.h
        Source/Rendering/RenderInfo.h
        Source/Chunk/Chunk.cpp
        Source/Chunk/Chunk.h
        Source/Chunk/VoxelWorld.cpp
        Source/Chunk/VoxelWorld.h
)

target_include_directories(SpireVoxel PUBLIC "Source/")
target_link_libraries(SpireVoxel PUBLIC Spire)

setup_assets_directory(SpireVoxel Assets)