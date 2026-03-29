#include "viewport_panel.hpp"
#include <imgui.h>

namespace mocap {

    ViewportPanel::ViewportPanel(CaptureThread& captureSystem, Texture& texture, AppState& state)
        : m_captureSystem(captureSystem), m_texture(texture), m_state(state) {}

    void ViewportPanel::render()
    {
        ImGui::Begin("Camera Feed");
        
        auto maybe_frame = m_captureSystem.getLatestFrame();
        if (maybe_frame.has_value())
        {
            m_texture.update(maybe_frame.value());
        }

        if (m_texture.isValid() && m_state != AppState::IDLE)
        {
            ImVec2 avail_size = ImGui::GetContentRegionAvail();
            float aspect = (float)m_texture.getWidth() / (float)m_texture.getHeight();
            
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

            ImTextureID tex_id = (ImTextureID)(intptr_t)m_texture.getId();
            ImGui::Image(tex_id, render_size);
        }
        else
        {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No active source.");
        }
        
        ImGui::End();
    }
}