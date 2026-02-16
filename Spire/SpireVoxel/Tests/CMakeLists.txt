include(../../CMake/Utils/Tests.cmake)

add_executable(SpireVoxelTests
        Tests/VertexPackingTests.cpp
        Tests/VoxelWorldTests.cpp
        Tests/TestVoxelEdits.cpp
        Tests/TestHelpers.h
        Tests/TestFaceUtils.cpp
        Tests/GreedyMeshingTests.cpp
        Tests/VoxelTypePackingTests.cpp
        Tests/AmbientOcclusionTests.cpp
)

target_include_directories(SpireVoxelTests PRIVATE "Tests/")
target_link_libraries(SpireVoxelTests PRIVATE SpireVoxel)

AddTests(SpireVoxelTests SpireVoxel)
