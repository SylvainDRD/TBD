#pragma once

#include <general/window.hpp>
#include <misc/types.hpp>
#include <misc/utils.hpp>

namespace TBD {

class VulkanRHI;

class Engine {
    TBD_NO_COPY_MOVE(Engine)
public:
    Engine();

    ~Engine();

    void run();

private:
    Window _window;

    Uptr<VulkanRHI> _rhi;
};

} // namespace TBD