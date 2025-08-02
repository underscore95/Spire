#pragma once

#include "Window.h"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <libassert/assert.hpp>

static void GLFW_KeyCallback(GLFWwindow *pWindow, int key, int scanCode, int action, int mods) {
    auto window = static_cast<Window *>(glfwGetWindowUserPointer(pWindow));

    window->KeyCallback(key, scanCode, action, mods);
}


static void GLFW_MouseCallback(GLFWwindow *pWindow, double xPos, double yPos) {
    auto window = static_cast<Window *>(glfwGetWindowUserPointer(pWindow));

    window->MouseCallback(xPos, yPos);
}

static void GLFW_MouseButtonCallback(GLFWwindow *pWindow, int button, int action, int mods) {
    auto window = static_cast<Window *>(glfwGetWindowUserPointer(pWindow));

    window->MouseButtonCallback(button, action, mods);
}

static void GLFW_ScrollCallback(GLFWwindow *pWindow, double xOffset, double yOffset) {
    auto window = static_cast<Window *>(glfwGetWindowUserPointer(pWindow));

    window->ScrollCallback(xOffset, yOffset);
}

Window::Window(const std::string &title, glm::ivec2 size) {
    ASSERT(s_initialized);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    m_windowHandle = glfwCreateWindow(size.x, size.y, title.data(), nullptr, nullptr);

    glfwSetWindowUserPointer(m_windowHandle, this);

    glfwSetKeyCallback(m_windowHandle, GLFW_KeyCallback);
    glfwSetCursorPosCallback(m_windowHandle, GLFW_MouseCallback);
    glfwSetMouseButtonCallback(m_windowHandle, GLFW_MouseButtonCallback);
    glfwSetScrollCallback(m_windowHandle, GLFW_ScrollCallback);
}

Window::~Window() {
    if (m_windowHandle) {
        glfwSetWindowUserPointer(m_windowHandle, nullptr);
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

bool Window::IsKeyHeld(int key) const {
    return IsKeyPressed(key) || m_heldKeys.contains(key);
}

bool Window::IsKeyPressed(int key) const {
    return m_pressedKeys.contains(key);
}

bool Window::IsMouseButtonHeld(int button) const {
    return IsMouseButtonPressed(button) || m_heldMouseButtons.contains(button);
}

bool Window::IsMouseButtonPressed(int button) const {
    return m_pressedMouseButtons.contains(button);
}

glm::vec2 Window::GetMousePos() const {
    return m_mousePos;
}

glm::vec2 Window::GetMouseDelta() const {
    return m_mouseDelta;
}

glm::vec2 Window::GetScrollDelta() const {
    return m_scrollDelta;
}

float Window::GetAspectRatio() const {
    const glm::vec2 size = GetDimensions();
    ASSERT(size.y != 0.0f);
    return size.x / size.y;
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

    spdlog::info("Initialized window!");
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
    spdlog::info("Shut down window");
}

bool Window::IsValid() const {
    return m_windowHandle != nullptr;
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_windowHandle);
}

GLFWwindow *Window::GLFWWindow() const {
    return m_windowHandle;
}

glm::uvec2 Window::GetDimensions() const {
    glm::ivec2 size;
    glfwGetWindowSize(m_windowHandle, &size.x, &size.y);
    return size;
}

void Window::Update() {
    ASSUME(s_initialized);

    // pressed -> held
    for (int key: m_pressedKeys) {
        m_heldKeys.insert(key);
    }
    m_pressedKeys.clear();

    for (int button: m_pressedMouseButtons) {
        m_heldMouseButtons.insert(button);
    }
    m_pressedMouseButtons.clear();

    // poll events
    glfwPollEvents();

    // mouse pos & delta
    m_mouseDelta = m_mouseRawPos - m_mousePos;
    m_mousePos = m_mouseRawPos;

    // mouse scroll
    m_scrollDelta = m_scrollDeltaTemp;
    m_scrollDeltaTemp = {0, 0};
}

void Window::KeyCallback(int key, int scanCode, int action, int mods) {
    if (action == GLFW_PRESS) {
        m_heldKeys.erase(key);
        m_pressedKeys.insert(key);
    } else if (action == GLFW_RELEASE) {
        m_heldKeys.erase(key);
        m_pressedKeys.erase(key);
    }
}

void Window::MouseCallback(double xPos, double yPos) {
    m_mouseRawPos = {xPos, yPos};
}

void Window::MouseButtonCallback(int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        m_heldMouseButtons.erase(button);
        m_pressedMouseButtons.insert(button);
    } else if (action == GLFW_RELEASE) {
        m_heldMouseButtons.erase(button);
        m_pressedMouseButtons.erase(button);
    }
}

void Window::ScrollCallback(double xOffset, double yOffset) {
    m_scrollDeltaTemp += glm::vec2{xOffset, yOffset};
}

bool Window::s_initialized = false;
