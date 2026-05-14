#include "scene_panel.hpp"
#include <imgui.h>
#include <glad/glad.h>

namespace mocap {

ScenePanel::ScenePanel(FittingThread& fittingThread)
    : m_fittingThread(fittingThread) 
{
}

void ScenePanel::initialize() 
{
    m_renderer.initialize();
    m_fbo.initialize(800, 600); 
}

void ScenePanel::handleInputs() 
{
    if (!ImGui::IsWindowHovered()) return;

    ImGuiIO& io = ImGui::GetIO();
    
    // orbit by lmb
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        m_camera.processMouseOrbit(io.MouseDelta.x, io.MouseDelta.y);
    }
    
    // pan by rmb
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        m_camera.processMousePan(io.MouseDelta.x, io.MouseDelta.y);
    }

    // zoom by wheel
    if (io.MouseWheel != 0.0f)
    {
        m_camera.processMouseScroll(io.MouseWheel);
    }
}

void ScenePanel::render(ApplicationState& /*state*/) 
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("3D Scene");

    // dynamic resizing
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    if (viewportSize.x > 0 && viewportSize.y > 0)
    {
        m_fbo.resize(static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));
        m_camera.setViewportSize(viewportSize.x, viewportSize.y);
    }

    // read mouse input
    handleInputs();

    // render 3d
    m_fbo.bind();
    glClearColor(0.12f, 0.12f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto poseOpt = m_fittingThread.getLatestPose();
    if (poseOpt.has_value()) {
        m_renderer.render(poseOpt.value(), m_camera);
    }

    m_fbo.unbind();

    // draw to imgui
    ImGui::Image(
        reinterpret_cast<void*>(static_cast<intptr_t>(m_fbo.getTextureID())), 
        viewportSize, 
        ImVec2(0, 1), 
        ImVec2(1, 0)
    );

    ImGui::End();
    ImGui::PopStyleVar();
}

}