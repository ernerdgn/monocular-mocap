#include "viewport_panel.hpp"

#include <imgui.h>

namespace mocap
{

static const std::vector<std::pair<BodyJoint, BodyJoint>> SKELETON_BONES = {
    {BodyJoint::NOSE, BodyJoint::LEFT_EYE}, {BodyJoint::LEFT_EYE, BodyJoint::LEFT_EAR},
    {BodyJoint::NOSE, BodyJoint::RIGHT_EYE}, {BodyJoint::RIGHT_EYE, BodyJoint::RIGHT_EAR},
    {BodyJoint::LEFT_SHOULDER, BodyJoint::RIGHT_SHOULDER},
    {BodyJoint::LEFT_SHOULDER, BodyJoint::LEFT_ELBOW}, {BodyJoint::LEFT_ELBOW, BodyJoint::LEFT_WRIST},
    {BodyJoint::RIGHT_SHOULDER, BodyJoint::RIGHT_ELBOW}, {BodyJoint::RIGHT_ELBOW, BodyJoint::RIGHT_WRIST},
    {BodyJoint::LEFT_SHOULDER, BodyJoint::LEFT_HIP}, {BodyJoint::RIGHT_SHOULDER, BodyJoint::RIGHT_HIP},
    {BodyJoint::LEFT_HIP, BodyJoint::RIGHT_HIP},
    {BodyJoint::LEFT_HIP, BodyJoint::LEFT_KNEE}, {BodyJoint::LEFT_KNEE, BodyJoint::LEFT_ANKLE},
    {BodyJoint::RIGHT_HIP, BodyJoint::RIGHT_KNEE}, {BodyJoint::RIGHT_KNEE, BodyJoint::RIGHT_ANKLE}
};

ViewportPanel::ViewportPanel(CaptureThread& captureSystem, DetectionThread& detectionSystem, Texture& texture, AppState& state)
    : m_captureSystem(captureSystem), m_detectionSystem(detectionSystem), m_texture(texture), m_state(state)
{
}

ImU32 ViewportPanel::getConfidenceColor(float confidence) const
{
    if (confidence > 0.7f) return IM_COL32(0, 255, 0, 255); //g
    if (confidence > 0.4f) return IM_COL32(255, 255, 0, 255); //y
    return IM_COL32(255, 0, 0, 255); //r
}

void ViewportPanel::render()
{
    ImGui::Begin("Camera Feed");

    auto maybe_frame = m_captureSystem.getLatestFrame();
    if (maybe_frame.has_value())
    {
        m_texture.update(maybe_frame.value()->image);
    }

    if (m_texture.isValid() && m_state != AppState::IDLE)
    {
        ImVec2 avail_size = ImGui::GetContentRegionAvail();
        float aspect      = (float) m_texture.getWidth() / (float) m_texture.getHeight();

        ImVec2 render_size;
        if (avail_size.x / aspect <= avail_size.y)
        {
            render_size.x = avail_size.x;
            render_size.y = avail_size.x / aspect;
        }
        else
        {
            render_size.y = avail_size.y;
            render_size.x = avail_size.y * aspect;
        }

        ImVec2 cursor_pos = ImGui::GetCursorPos();
        cursor_pos.x += (avail_size.x - render_size.x) * 0.5f;
        cursor_pos.y += (avail_size.y - render_size.y) * 0.5f;
        ImGui::SetCursorPos(cursor_pos);

        ImTextureID tex_id = (ImTextureID) (intptr_t) m_texture.getId();
        ImGui::Image(tex_id, render_size);

        // overlay renderer
        ImVec2 p_min = ImGui::GetItemRectMin(); // top left
        ImVec2 p_max = ImGui::GetItemRectMax(); // bottom right
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        auto maybe_result = m_detectionSystem.getLatestResult();
        if (maybe_result.has_value()) 
        {
            m_lastResult = maybe_result.value();
            m_hasResult = true;
        }

        if (m_hasResult) 
        {
            const float CONF_THRESHOLD = 0.3f;

            // draw lines (bones)
            for (const auto& bone : SKELETON_BONES) 
            {
                // Use m_lastResult instead of the direct queue result
                const auto& j1 = m_lastResult.bodyJoints[static_cast<int>(bone.first)];
                const auto& j2 = m_lastResult.bodyJoints[static_cast<int>(bone.second)];

                if (j1.confidence > CONF_THRESHOLD && j2.confidence > CONF_THRESHOLD) 
                {
                    ImVec2 pos1 = ImVec2(p_min.x + j1.position.x * render_size.x, p_min.y + j1.position.y * render_size.y);
                    ImVec2 pos2 = ImVec2(p_min.x + j2.position.x * render_size.x, p_min.y + j2.position.y * render_size.y);
                    draw_list->AddLine(pos1, pos2, IM_COL32(200, 200, 200, 200), 2.0f);
                }
            }

            // draw circles (joints)
            for (const auto& joint : m_lastResult.bodyJoints) 
            {
                if (joint.confidence > CONF_THRESHOLD) 
                {
                    ImVec2 pos = ImVec2(p_min.x + joint.position.x * render_size.x, p_min.y + joint.position.y * render_size.y);
                    ImU32 color = getConfidenceColor(joint.confidence);
                    
                    float radius = 3.0f + (joint.confidence * 3.0f); 
                    
                    draw_list->AddCircleFilled(pos, radius, color);
                    draw_list->AddCircle(pos, radius + 1.0f, IM_COL32(0, 0, 0, 255)); 
                }
            }
        }
    }

    else
    {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No active source.");
    }

    ImGui::End();
}
}