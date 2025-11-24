include(../../CMake/Utils/Tests.cmake)

add_executable(SpireVoxelTests
        Tests/VertexPackingTests.cpp
        Tests/VoxelWorldTests.cpp
        Tests/TestVoxelEdits.cpp
        Tests/TestHelpers.h
)

target_include_directories(SpireVoxelTests PRIVATE "Tests/")
target_link_libraries(SpireVoxelTests PRIVATE SpireVoxel)

AddTests(SpireVoxelTests SpireVoxel)
