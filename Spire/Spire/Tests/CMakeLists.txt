include(../../CMake/Utils/Tests.cmake)

add_executable(SpireTests
        Tests/Test.cpp
)

target_include_directories(SpireTests PRIVATE "Tests/")
target_link_libraries(SpireTests PRIVATE Spire)

AddTests(SpireTests Spire)
