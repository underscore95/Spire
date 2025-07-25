cmake_minimum_required(VERSION 3.31)
project(VulkanTutorial)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(VulkanTutorial src/main.cpp src/hello.cpp)

target_sources(VulkanTutorial PRIVATE FILE_SET CXX_MODULES FILES src/hello.ixx)

target_link_libraries(VulkanTutorial
        PRIVATE
        ${VULKAN_LIB_LIST}
        glfw
        glm::glm-header-only
)

target_include_directories(VulkanTutorial
        PUBLIC ${VULKAN_INCLUDE_DIRS}
)
