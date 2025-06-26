#pragma once

#include "renderer/vulkan/vulkan_rhi.hpp"
#include <general/window.hpp>
#include <misc/utils.hpp>

namespace TBD {

class Engine {
    TBD_NO_COPY_MOVE(Engine)
public:
    Engine();

    ~Engine();

    void run();

private:
    Window _window;

    VulkanRHI *_rhi;
};

} // namespace TBD