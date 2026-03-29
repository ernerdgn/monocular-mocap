#pragma once

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
    virtual void render() = 0;
};

} // namespace mocap