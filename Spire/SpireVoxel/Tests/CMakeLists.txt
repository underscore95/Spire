include(../../CMake/Utils/Tests.cmake)

add_executable(SpireVoxelTests
        Tests/Test.cpp
        Tests/VertexPackingTests.cpp
)

target_include_directories(SpireVoxelTests PRIVATE "Tests/")
target_link_libraries(SpireVoxelTests PRIVATE SpireVoxel)

AddTests(SpireVoxelTests SpireVoxel)
