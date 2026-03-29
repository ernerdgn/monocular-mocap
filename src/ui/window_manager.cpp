#include "window_manager.hpp"
#include "core/logger.hpp"
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

namespace mocap {
    WindowManager::WindowManager() : m_window(nullptr), m_isInitialized(false) {}

    WindowManager::~WindowManager()
    {
        shutdown();
    }

    Result<void> WindowManager::initialize(int width, int height, const std::string& title)
    {
        MOCAP_INFO("Initializing GLFW and OpenGL...");

        if (!glfwInit()) return Result<void>::fail("Failed to initialize GLFW.");

        // ogl4.5 core profile
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

        if (!m_window)
        {
            glfwTerminate();
            return Result<void>::fail("Failed to create GLFW window.");
        }

        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1);  // vsync

        // glad init
        MOCAP_INFO("Loading OpenGL functions via GLAD...");
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            return Result<void>::fail("Failed to initialize GLAD.");
        }

        // init dearest imgui
        MOCAP_INFO("Initializing Dear ImGui...");
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // key controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // docking

        // style
        ImGui::StyleColorsDark();

        // setup platform/renderer backends
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);

        // tell imgui which version of ogl
        ImGui_ImplOpenGL3_Init("#version 450");

        m_isInitialized = true;
        MOCAP_INFO("Window and UI initialized successfully.");
        return Result<void>::ok();
    }

    void WindowManager::shutdown()
    {
        if (m_isInitialized)
        {
            MOCAP_INFO("Shutting down UI...");
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();

            glfwDestroyWindow(m_window);
            glfwTerminate();
            m_isInitialized = false;
        }
    }

    bool WindowManager::shouldClose() const
    {
        return m_isInitialized && glfwWindowShouldClose(m_window);
    }

    void WindowManager::beginFrame() 
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Enable the root docking space
        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
    }

    void WindowManager::endFrame()
    {
        ImGui::Render();

        // clear context
        int display_w, display_h;
        glfwGetFramebufferSize(m_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw imgui geo
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
    }
}