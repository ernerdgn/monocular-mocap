#pragma once
#include "core/types.hpp"
#include "core/result.hpp"
#include "render/orbital_camera.hpp"
#include <glad/glad.h>
#include <vector>

namespace mocap {

class SkeletonRenderer 
{
public:
    SkeletonRenderer();
    ~SkeletonRenderer();

    Result<void> initialize();

    void render(const PoseFrame& pose, const OrbitalCamera& camera);

private:
    bool m_isInitialized = false;

    GLuint m_shaderProgram = 0;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;

    Result<void> compileShaders();

    std::vector<std::pair<int, int>> m_boneConnections;
};

}