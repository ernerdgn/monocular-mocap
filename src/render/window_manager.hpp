#pragma once
#include "core/result.hpp"

#include <string>

struct GLFWwindow;

namespace mocap
{
class WindowManager
{
  public:
    WindowManager();
    ~WindowManager();

    // init glfw, ogl4.5 and imgui
    Result<void> initialize(int width, int height, const std::string& title);

    // cleanup
    void shutdown();

    // exit check
    bool shouldClose() const;

    // imgui frame start
    void beginFrame();

    // render imgui to ogl buffer and swap to display
    void endFrame();

    GLFWwindow* getHandle() const
    {
        return m_window;
    }

  private:
    GLFWwindow* m_window;
    bool m_isInitialized;
};
} // namespace mocap