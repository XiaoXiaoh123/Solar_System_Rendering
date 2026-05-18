#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

Camera::Camera(const glm::vec3& position, float yaw, float pitch)
    : m_position(position), m_yaw(yaw), m_pitch(pitch)
{
    updateVectors();
}

void Camera::updateVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up    = glm::normalize(glm::cross(m_right, m_front));
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(m_fov), aspectRatio, m_nearPlane, m_farPlane);
}

void Camera::moveForward(float amount) {
    m_position += m_front * amount;
}

void Camera::moveRight(float amount) {
    m_position += m_right * amount;
}

void Camera::moveUp(float amount) {
    m_position += m_worldUp * amount;
}

void Camera::rotate(float yawDelta, float pitchDelta) {
    m_yaw   += yawDelta;
    m_pitch += pitchDelta;
    m_pitch  = glm::clamp(m_pitch, -89.0f, 89.0f);
    updateVectors();
}

void Camera::zoom(float amount) {
    m_fov = glm::clamp(m_fov - amount, 1.0f, 90.0f);
}
