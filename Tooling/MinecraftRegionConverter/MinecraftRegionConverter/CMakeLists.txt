cmake_minimum_required(VERSION 3.31)
project(mre)

set(CMAKE_CXX_STANDARD 20)

add_executable(mre main.cpp
        miniz.cpp
        miniz.h
        enkimi.cpp
        enkimi.h
        minecraft.cpp
        minecraft.h
        utils.h)

include(FetchContent)

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json)

target_link_libraries(mre PRIVATE nlohmann_json::nlohmann_json)