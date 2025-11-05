
add_executable(Game
        Source/Main.cpp
        Source/GameApplication.cpp
        Source/GameApplication.h
)

target_include_directories(Game PRIVATE "Source/")
target_link_libraries(Game PRIVATE SpireVoxel)

setup_assets_directory(Game Assets)
