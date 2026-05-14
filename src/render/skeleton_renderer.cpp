#include "skeleton_renderer.hpp"
#include "core/logger.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace mocap {

// embedded glsl shaders (ver330core)
const char* VERTEX_SHADER_SRC = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
}
)";

const char* FRAGMENT_SHADER_SRC = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.2, 0.8, 0.2, 1.0); // Neon Green for the skeleton
}
)";

SkeletonRenderer::SkeletonRenderer() 
{
    m_boneConnections = {
        {0, 1}, {0, 2}, {1, 3}, {2, 4},       // head
        {5, 6}, {5, 11}, {6, 12}, {11, 12},   // torso
        {5, 7}, {7, 9},                       // left arm
        {6, 8}, {8, 10},                      // right arm
        {11, 13}, {13, 15},                   // left leg
        {12, 14}, {14, 16}                    // right leg
    };
}

SkeletonRenderer::~SkeletonRenderer() 
{
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
}

Result<void> SkeletonRenderer::compileShaders() 
{
    // compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &VERTEX_SHADER_SRC, NULL);
    glCompileShader(vertexShader);

    // compile pixel shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &FRAGMENT_SHADER_SRC, NULL);
    glCompileShader(fragmentShader);

    // link
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);

    // cleanup
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return Result<void>();
}

Result<void> SkeletonRenderer::initialize() 
{
    auto shaderRes = compileShaders();
    if (!shaderRes.is_ok()) return shaderRes;

    // vao and vbo
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_isInitialized = true;
    return Result<void>();
}

void SkeletonRenderer::render(const PoseFrame& pose, const OrbitalCamera& camera) 
{
    if (!m_isInitialized || pose.body_joints.empty()) return;

    // prep geo data
    std::vector<float> lineVertices;
    lineVertices.reserve(m_boneConnections.size() * 6); // 2 points per bone, 3 floats per point

    for (const auto& bone : m_boneConnections) 
    {
        int j1 = bone.first;
        int j2 = bone.second;

        // is model output matches joint map?.. i hope so
        if (j1 < pose.body_joints.size() && j2 < pose.body_joints.size()) 
        {
            glm::vec3 p1 = pose.body_joints[j1].position + pose.root_translation;
            glm::vec3 p2 = pose.body_joints[j2].position + pose.root_translation;

            lineVertices.insert(lineVertices.end(), {p1.x, p1.y, p1.z});
            lineVertices.insert(lineVertices.end(), {p2.x, p2.y, p2.z});
        }
    }

    if (lineVertices.empty()) return;

    // debug flag
    static int render_debug_count = 0;
    if (render_debug_count < 5)
    {
        MOCAP_INFO("DEBUG [Render]: Generating {} vertices for the GPU. First vertex: ({}, {}, {})", 
            lineVertices.size(),
            lineVertices[0], lineVertices[1], lineVertices[2]);
        render_debug_count++;
    }
    // --------------------------

    // gl state
    glUseProgram(m_shaderProgram);
    glEnable(GL_DEPTH_TEST); // Ensure 3D depth sorting is active
    
    // line thickens...
    glLineWidth(2.0f); 

    // upload cam mat
    GLuint viewLoc = glGetUniformLocation(m_shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(m_shaderProgram, "projection");
    
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix()));

    // upload geo and draw
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    
    // GL_DYNAMIC_DRAW because the skeleton moves every single frame
    glBufferData(GL_ARRAY_BUFFER, lineVertices.size() * sizeof(float), lineVertices.data(), GL_DYNAMIC_DRAW);
    
    // draw lines
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lineVertices.size() / 3));

    // cleanup
    glBindVertexArray(0);
    glUseProgram(0);
}

}