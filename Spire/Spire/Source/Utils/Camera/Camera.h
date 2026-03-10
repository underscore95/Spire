#pragma once

#include "Frustum.h"
#include "pch.h"

namespace Spire {
    class Window;

    class Camera {
    public:
        enum class ControlScheme {
            Default, Developer
        };

    public:
        explicit Camera(
            ControlScheme controlScheme,
            Window &m_window,
            glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
            float yawDegrees = YAW,
            float pitchDegrees = PITCH,
            float fovYDegrees = 45
        );

    public:
        [[nodiscard]] glm::mat4 GetViewMatrix() const;

        [[nodiscard]] glm::mat4 GetProjectionMatrix() const;

        void Update(float deltaTime);

        [[nodiscard]] glm::vec3 GetForward() const;

        [[nodiscard]] glm::vec3 GetPosition() const;

        void SetPosition(glm::vec3 pos);

        [[nodiscard]] Frustum CalculateFrustum() const;

        void SetYawPitch(float yawDegrees, float pitchDegrees);

    private:
        void UpdateCameraVectors();

    private:
        // YAW & PITCH are degrees
        static constexpr float YAW = -90.0f;
        static constexpr float PITCH = 0.0f;
        static constexpr float SPEED = 25.0f;
        static constexpr float SENSITIVITY = 0.05f;
        static constexpr bool CONSTRAIN_PITCH = true; // if true, pitch range -89 to 89 to prevent screen flipping

    private:
        Window &m_window;

        glm::vec3 m_position;
        glm::vec3 m_front;
        glm::vec3 m_up;
        glm::vec3 m_left;
        glm::vec3 m_worldUp;

        float m_yawDegrees;
        float m_pitchDegrees;

        float m_movementSpeed;
        float m_mouseSensitivity;
        float m_fovY;
        ControlScheme m_controlScheme;
    };
}
