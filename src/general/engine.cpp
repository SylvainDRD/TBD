#include "engine.hpp"
#include <memory>
#include <renderer/rendering_dag/rendering_dag.hpp>
#include <renderer/vulkan/vulkan_rhi.hpp>

namespace TBD {

Engine::Engine()
    : _window {}
{
    _rhi = std::make_unique<VulkanRHI>(_window);
}

Engine::~Engine() { }

void Engine::run()
{
    while (!_window.windowClosing()) {

        _rhi->render(RenderingDAG {});
        _window.update();
    }
}

} // namespace TBD