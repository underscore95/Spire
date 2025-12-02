
add_library(SpireVoxel STATIC
        Source/VoxelRenderer.cpp
        Source/VoxelRenderer.h
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
        Source/Edits/IVoxelEdit.h
        Source/Edits/MergedVoxelEdit.cpp
        Source/Edits/MergedVoxelEdit.h
        Source/Edits/BasicVoxelEdit.cpp
        Source/Edits/BasicVoxelEdit.h
        Source/Edits/CuboidVoxelEdit.cpp
        Source/Edits/CuboidVoxelEdit.h
        Source/SpireVoxelRenderer.h
        Source/Utils/RaycastUtils.cpp
        Source/Utils/RaycastUtils.h
        Source/Utils/IVoxelCamera.h
        Source/Chunk/GreedyMeshingGrid.h
        Source/Chunk/GreedyMeshingGrid.cpp
        Source/Chunk/VoxelPositionToTypeHashMap.cpp
        Source/Chunk/VoxelPositionToTypeHashMap.h
)

target_include_directories(SpireVoxel PUBLIC "Source/")
target_link_libraries(SpireVoxel PUBLIC Spire)

setup_assets_directory(SpireVoxel Assets)

# Tests
add_subdirectory(Tests)