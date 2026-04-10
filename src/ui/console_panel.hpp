#pragma once
#include "ui_types.hpp"
#include "application_state.hpp"

namespace mocap
{

class ConsolePanel : public IPanel
{
  public:
    ConsolePanel();
    void render(ApplicationState& state) override;

  private:
    bool m_autoScroll;

    bool m_showTrace;
    bool m_showInfo;
    bool m_showWarn;
    bool m_showError;
};

} // namespace mocap