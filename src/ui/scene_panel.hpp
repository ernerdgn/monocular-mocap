#pragma once
#include "ui_types.hpp"
#include "render/framebuffer.hpp"
#include "render/orbital_camera.hpp"
#include "render/skeleton_renderer.hpp"
#include "fitting/fitting_thread.hpp"

namespace mocap {

class ScenePanel : public IPanel 
{
public:
    explicit ScenePanel(FittingThread& fittingThread);
    ~ScenePanel() = default;

    void initialize();

    void render(ApplicationState& state) override;

private:
    FittingThread& m_fittingThread;
    
    Framebuffer m_fbo;
    OrbitalCamera m_camera;
    SkeletonRenderer m_renderer;

    void handleInputs();
};

} // namespace mocap