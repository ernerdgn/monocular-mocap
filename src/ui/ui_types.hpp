#pragma once
#include "application_state.hpp"

namespace mocap
{

// enum class AppState
// {
//     IDLE,
//     CAPTURING,
//     REVIEWING
// };

class IPanel
{
  public:
    virtual ~IPanel()     = default;
    virtual void render(ApplicationState& state) = 0;
};

} // namespace mocap