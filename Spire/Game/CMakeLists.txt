
add_executable(Game
    Source/Main.cpp
)

target_include_directories(Game PRIVATE "Source/")
target_link_libraries(Game PRIVATE Spire)