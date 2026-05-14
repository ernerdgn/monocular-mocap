#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace mocap {

class OrbitalCamera 
{
public:
    OrbitalCamera();
    ~OrbitalCamera() = default;

    void setViewportSize(float width, float height);

    void processMouseOrbit(float deltaX, float deltaY);
    void processMousePan(float deltaX, float deltaY);
    void processMouseScroll(float yOffset);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

private:
    // cam state
    glm::vec3 m_target;
    float m_distance;
    float m_yaw;
    float m_pitch;

    // lens state
    float m_fov;
    float m_aspectRatio;
    float m_nearPlane;
    float m_farPlane;

    glm::vec3 calculateCameraPosition() const;
};

}