#pragma once
#include "core/result.hpp"
#include <string>

struct GLFWwindow;

namespace mocap {
    class WindowManager
    {
        public:
        WindowManager();
        ~WindowManager();

        // init glfw, ogl4.5 and imgui
        Result<void> Initialize(int width, int height, const std::string& title);

        // cleanup
        void Shutdown();

        // exit check
        bool ShouldClose() const;
        
        // imgui frame start
        void BeginFrame();
        
        // render imgui to ogl buffer and swap to display
        void EndFrame();

        GLFWwindow* GetHandle() const { return m_window; }

        private:
        GLFWwindow* m_window;
        bool m_isInitialized;
    };
}