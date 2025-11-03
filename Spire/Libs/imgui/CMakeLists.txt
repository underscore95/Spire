
file(GLOB IMGUI_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.cpp"
)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
)

target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/")
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../glfw/include/")

find_package(Vulkan REQUIRED)
target_link_libraries(imgui PRIVATE Vulkan::Vulkan)