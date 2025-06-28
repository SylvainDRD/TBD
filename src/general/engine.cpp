#include "engine.hpp"
#include "renderer/vulkan/vulkan_rhi.hpp"

namespace TBD {

Engine::Engine()
    : _window {}
{
    _rhi = new VulkanRHI { _window };
}

Engine::~Engine()
{
    delete _rhi;
}

void Engine::run()
{
    while (!_window.windowClosing()) {
        
        _rhi->render();
        _window.update();
    }
}

} // namespace TBD