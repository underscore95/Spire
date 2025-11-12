#pragma once

// std
#include <memory>
#include <chrono>
#include <string>
#include <array>
#include <vector>
#include <fstream>
#include <unordered_set>
#include <filesystem>
#include <atomic>
#include <condition_variable>
#include <unordered_map>
#include <format>
#include <cassert>
#include <sys/stat.h>
#include <iostream>
#include <chrono>
#include <optional>
#include <functional>
#include <regex>
#include <map>

// vulkan
#include <vulkan/vulkan_core.h>

// vma
#include <vk_mem_alloc.h>

// imgui
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>