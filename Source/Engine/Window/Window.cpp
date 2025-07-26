#pragma once

#include "Window.h"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <libassert/assert.hpp>

Window::Window(const std::string &title, glm::ivec2 size) {
    ASSERT(s_initialized);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    m_windowHandle = glfwCreateWindow(size.x, size.y, title.data(), nullptr, nullptr);
}

Window::~Window() {
    if (m_windowHandle) {
        glfwDestroyWindow(m_windowHandle);
    }
}

std::unique_ptr<Window> Window::Create(const std::string &title, glm::ivec2 size) {
    if (!s_initialized) {
        spdlog::error("Failed to create window because Window::Init wasn't called.");
        return nullptr;
    }

    return std::unique_ptr<Window>(new Window(title, size));
}

bool Window::Init() {
    if (s_initialized) {
        spdlog::error("Window failed to initialize: already initialized");
        return false;
    }

    if (!glfwInit()) {
        spdlog::error("Window failed to initialize: glfwInit failed");
        return false;
    }

    if (!glfwVulkanSupported()) {
        spdlog::error("Window failed to initialize: GLFW doesn't support Vulkan");
        return false;
    }

    s_initialized = true;
    return true;
}

void Window::Shutdown() {
    if (!s_initialized) {
        spdlog::error("Window failed to shutdown: not initialized");
        return;
    }

    glfwTerminate();
    s_initialized = false;
}

bool Window::IsValid() const {
    return m_windowHandle != nullptr;
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_windowHandle);
}

void Window::Update() {
    ASSUME(s_initialized);

    glfwPollEvents();
}

bool Window::s_initialized = false;