#include "orbital_camera.hpp"
#include <algorithm>
#include <cmath>

namespace mocap {

OrbitalCamera::OrbitalCamera()
    : m_target(0.0f, 0.0f, 0.0f),
      m_distance(3.0f),
      m_yaw(0.0f),
      m_pitch(0.2f),
      m_fov(glm::radians(45.0f)),
      m_aspectRatio(16.0f / 9.0f),
      m_nearPlane(0.1f),
      m_farPlane(100.0f)
{
}

void OrbitalCamera::setViewportSize(float width, float height) 
{
    if (height > 0.0f)
    {
        m_aspectRatio = width / height;
    }
}

void OrbitalCamera::processMouseOrbit(float deltaX, float deltaY) 
{
    const float rotationSpeed = 0.01f;
    m_yaw -= deltaX * rotationSpeed;
    m_pitch += deltaY * rotationSpeed;

    const float maxPitch = glm::radians(89.0f);
    m_pitch = std::clamp(m_pitch, -maxPitch, maxPitch);
}

void OrbitalCamera::processMousePan(float deltaX, float deltaY) 
{
    const float panSpeed = 0.002f * m_distance;
    
    glm::vec3 camPos = calculateCameraPosition();
    glm::vec3 forward = glm::normalize(m_target - camPos);
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up = glm::normalize(glm::cross(right, forward));

    m_target -= right * (deltaX * panSpeed);
    m_target += up * (deltaY * panSpeed);
}

void OrbitalCamera::processMouseScroll(float yOffset) 
{
    const float zoomSpeed = 0.5f;
    m_distance -= yOffset * zoomSpeed;
    
    m_distance = std::clamp(m_distance, 0.5f, 20.0f);
}

glm::vec3 OrbitalCamera::calculateCameraPosition() const 
{
    float horizontalDistance = m_distance * cos(m_pitch);
    float verticalDistance = m_distance * sin(m_pitch);

    float x = m_target.x - horizontalDistance * sin(m_yaw);
    float y = m_target.y + verticalDistance;
    float z = m_target.z - horizontalDistance * cos(m_yaw);

    return glm::vec3(x, y, z);
}

glm::mat4 OrbitalCamera::getViewMatrix() const 
{
    glm::vec3 position = calculateCameraPosition();
    return glm::lookAt(position, m_target, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 OrbitalCamera::getProjectionMatrix() const 
{
    return glm::perspective(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
}

}