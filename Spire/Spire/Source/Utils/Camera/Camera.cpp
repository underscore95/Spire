#include "Camera.h"
#include <GLFW/glfw3.h>
#include "Window/Window.h"

// Adapted from: https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/camera.h

namespace Spire {
    Camera::Camera(
        ControlScheme controlScheme,
        Window &m_window,
        glm::vec3 position,
        glm::vec3 up,
        float yawDegrees,
        float pitchDegrees,
        float fovYDegrees)
        : m_window(m_window),
          m_position(position),
          m_front(glm::vec3(0.0f, 0.0f, -1.0f)),
          m_worldUp(up),
          m_yawDegrees(yawDegrees),
          m_pitchDegrees(pitchDegrees),
          m_movementSpeed(SPEED),
          m_mouseSensitivity(SENSITIVITY),
          m_fovY(glm::radians(fovYDegrees)),
          m_controlScheme(controlScheme) {
        UpdateCameraVectors();
    }

    glm::mat4 Camera::GetViewMatrix() const {
        return glm::lookAt(m_position, m_position + m_front, -m_up);
    }

    glm::mat4 Camera::GetProjectionMatrix() const {
        return glm::perspective(m_fovY, m_window.GetAspectRatio(), 0.1f, 10000.0f);
    }

    void Camera::Update(float deltaTime) {
        float velocity = m_movementSpeed * deltaTime;
        if (m_controlScheme == ControlScheme::Developer) {
            // Keyboard
            if (m_window.IsKeyHeld(GLFW_KEY_W))
                m_position += m_front * velocity;
            if (m_window.IsKeyHeld(GLFW_KEY_S))
                m_position -= m_front * velocity;
            if (m_window.IsKeyHeld(GLFW_KEY_A))
                m_position += m_left * velocity;
            if (m_window.IsKeyHeld(GLFW_KEY_D))
                m_position -= m_left * velocity;
            if (m_window.IsKeyHeld(GLFW_KEY_Q))
                m_position -= m_up * velocity;
            if (m_window.IsKeyHeld(GLFW_KEY_E))
                m_position += m_up * velocity;

            // Mouse
            if (m_window.IsMouseButtonHeld(GLFW_MOUSE_BUTTON_RIGHT)) {
                m_yawDegrees -= m_window.GetMouseDelta().x * m_mouseSensitivity;
                m_pitchDegrees -= m_window.GetMouseDelta().y * m_mouseSensitivity;
            }
        } else if (m_controlScheme == ControlScheme::Default) {
            glm::vec3 front = glm::normalize(glm::vec3{m_front.x, 0, m_front.z});
            glm::vec3 left = glm::normalize(glm::vec3{m_left.x, 0, m_left.z});
            glm::vec3 up = m_worldUp;

            // Keyboard
            if (m_window.IsKeyHeld(GLFW_KEY_W))
                m_position += front * velocity;
            if (m_window.IsKeyHeld(GLFW_KEY_S))
                m_position -= front * velocity;
            if (m_window.IsKeyHeld(GLFW_KEY_A))
                m_position += left * velocity;
            if (m_window.IsKeyHeld(GLFW_KEY_D))
                m_position -= left * velocity;
            if (m_window.IsKeyHeld(GLFW_KEY_LEFT_SHIFT))
                m_position -= up * velocity;
            if (m_window.IsKeyHeld(GLFW_KEY_SPACE))
                m_position += up * velocity;

            // Mouse
            m_yawDegrees -= m_window.GetMouseDelta().x * m_mouseSensitivity;
            m_pitchDegrees -= m_window.GetMouseDelta().y * m_mouseSensitivity;

            m_window.SetCursorPos(m_window.GetDimensions() / 2u);
        } else
            assert(false);

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (CONSTRAIN_PITCH) {
            if (m_pitchDegrees > 89.0f)
                m_pitchDegrees = 89.0f;
            if (m_pitchDegrees < -89.0f)
                m_pitchDegrees = -89.0f;
        }

        // Update
        UpdateCameraVectors();
    }

    glm::vec3 Camera::GetForward() const {
        return m_front;
    }

    glm::vec3 Camera::GetPosition() const {
        return m_position;
    }

    void Camera::SetPosition(glm::vec3 pos) {
        m_position = pos;
        UpdateCameraVectors();
    }

    Frustum Camera::CalculateFrustum() const {
        return {GetProjectionMatrix() * GetViewMatrix()};
    }

    void Camera::SetYawPitch(float yawDegrees, float pitchDegrees) {
        m_yawDegrees = yawDegrees;
        m_pitchDegrees = pitchDegrees;
    }

    void Camera::UpdateCameraVectors() {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(m_yawDegrees)) * cos(glm::radians(m_pitchDegrees));
        front.y = sin(glm::radians(m_pitchDegrees));
        front.z = sin(glm::radians(m_yawDegrees)) * cos(glm::radians(m_pitchDegrees));
        m_front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        m_left = glm::normalize(glm::cross(m_front, m_worldUp));
        // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        m_up = glm::normalize(glm::cross(m_left, m_front));
    }
}
