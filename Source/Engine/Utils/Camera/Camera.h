#pragma once

#include <glm/glm.hpp>

class Window;

class Camera {
public:
    explicit Camera(const Window &m_window, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                    float yawDegrees = YAW, float pitchDegrees = PITCH);

public:
    glm::mat4 GetViewMatrix() const;

    glm::mat4 GetProjectionMatrix() const;

    void Update(float deltaTime);

private:
    void UpdateCameraVectors();

private:
    // YAW & PITCH are degrees
    static constexpr float YAW = -90.0f;
    static constexpr float PITCH = 0.0f;
    static constexpr float SPEED = 25.0f;
    static constexpr float SENSITIVITY = 0.05f;
    static constexpr float ZOOM = 45.0f;
    static constexpr bool CONSTRAIN_PITCH = true; // if true, pitch range -89 to 89 to prevent screen flipping

private:
    const Window &m_window;

    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;

    float m_yawDegrees;
    float m_pitchDegrees;

    float m_movementSpeed;
    float m_mouseSensitivity;
    float m_zoom;
};
