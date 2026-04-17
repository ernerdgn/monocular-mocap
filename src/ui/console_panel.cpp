#include "console_panel.hpp"
#include "core/logger.hpp"
#include <imgui.h>

namespace mocap {

ConsolePanel::ConsolePanel() 
    : m_autoScroll(true), m_showTrace(false), m_showInfo(true), m_showWarn(true), m_showError(true) {}

void ConsolePanel::render(ApplicationState& /*state*/)
{
    ImGui::Begin("Console");

    if (ImGui::Button("Clear"))
    {
        Logger::clearLogs();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_autoScroll);
    
    // filter checkboxes
    ImGui::SameLine(); ImGui::Text("  Filters:");
    ImGui::SameLine(); ImGui::Checkbox("Trace", &m_showTrace);
    ImGui::SameLine(); ImGui::Checkbox("Info", &m_showInfo);
    ImGui::SameLine(); ImGui::Checkbox("Warn", &m_showWarn);
    ImGui::SameLine(); ImGui::Checkbox("Error", &m_showError);
    ImGui::Separator();

    ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    auto logs = Logger::getLogs();
    for (const auto& log : logs)
    {
        // filter logic
        if (!m_showTrace && log.level == spdlog::level::trace) continue;
        if (!m_showInfo && log.level == spdlog::level::info) continue;
        if (!m_showWarn && log.level == spdlog::level::warn) continue;
        if (!m_showError && (log.level == spdlog::level::err || log.level == spdlog::level::critical)) continue;

        // color coding
        ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default white
        bool hasColor = false;
        
        if (log.level == spdlog::level::trace) { color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f); hasColor = true; }
        else if (log.level == spdlog::level::info) { color = ImVec4(0.4f, 0.8f, 0.4f, 1.0f); hasColor = true; }
        else if (log.level == spdlog::level::warn) { color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f); hasColor = true; }
        else if (log.level == spdlog::level::err || log.level == spdlog::level::critical) { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); hasColor = true; }

        if (hasColor) ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(log.text.c_str());
        if (hasColor) ImGui::PopStyleColor();
    }

    if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::End();
}

}