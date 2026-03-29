#include "console_panel.hpp"
#include "core/logger.hpp"
#include <imgui.h>

namespace mocap {

    ConsolePanel::ConsolePanel() : m_autoScroll(true) {}

    void ConsolePanel::render()
    {
        ImGui::Begin("Console");

        if (ImGui::Button("Clear"))
        {
            Logger::clearLogs();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &m_autoScroll);
        ImGui::Separator();

        // scrolling
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        auto logs = Logger::getLogs();
        for (const auto& log : logs)
        {
            ImGui::TextUnformatted(log.c_str());
        }

        // lock bottom if auto-scroll enabled
        if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();
        ImGui::End();
    }

}