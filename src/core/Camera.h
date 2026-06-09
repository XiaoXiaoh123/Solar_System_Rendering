#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    Camera(const glm::vec3& position = glm::vec3(0.0f, 200.0f, 500.0f),
           float yaw = -90.0f, float pitch = -30.0f);

    glm::mat4 getViewMatrix()       const;
    glm::mat4 getProjectionMatrix(float aspectRatio) const;
    glm::vec3 getPosition()         const { return m_position; }
    glm::vec3 getFront()            const { return m_front; }
    float     getFov()              const { return m_fov; }
    float     getNearPlane()        const { return m_nearPlane; }
    float     getFarPlane()         const { return m_farPlane; }

    void setPosition(const glm::vec3& pos) { m_position = pos; }
    void setYaw(float yaw)                 { m_yaw = yaw; updateVectors(); }
    void setPitch(float pitch)             { m_pitch = glm::clamp(pitch, -89.0f, 89.0f); updateVectors(); }
    void setFov(float fov)                 { m_fov = glm::clamp(fov, 1.0f, 90.0f); }

    void moveForward(float amount);
    void moveRight(float amount);
    void moveUp(float amount);
    void lookAt(const glm::vec3& target);
    void rotate(float yawDelta, float pitchDelta);
    void zoom(float amount);

private:
    void updateVectors();

    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float m_yaw;
    float m_pitch;
    float m_fov        = 45.0f;
    float m_nearPlane  = 0.1f;
    float m_farPlane   = 10000.0f;
};
