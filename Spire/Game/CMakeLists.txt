
add_executable(Game
    Source/Main.cpp
        Source/GameApplication.cpp
        Source/GameApplication.h
        Source/GameCamera.cpp
        Source/GameCamera.h
        Source/Rendering/ObjectRenderer.cpp
        Source/Rendering/ObjectRenderer.h
        Source/Rendering/RenderInfo.h
)

target_include_directories(Game PRIVATE "Source/")
target_link_libraries(Game PRIVATE Spire)

setup_assets_directory(Game Assets)
