#include "window.hpp"
#include "GLFW/glfw3.h"
#include "misc/utils.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

namespace TBD {

Window::Window()
    : _width { 1600 }
    , _height { 800 }
{
    if (!glfwInit()) {
        ABORT("GLFW init failed");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO
    _window = glfwCreateWindow(_width, _height, PROJECT_NAME, nullptr, nullptr);

    if (!_window) {
        ABORT("Window creation failed");
    }

    TBD_LOG("Window creation completed");
}

Window::~Window()
{
    glfwDestroyWindow(_window);
    glfwTerminate();
}

[[nodiscard]] std::vector<const char*> Window::requiredVulkanExtensions() const
{
    std::vector<const char*> exts {};

    uint32_t count;
    const char** rawExts = glfwGetRequiredInstanceExtensions(&count);

    for (int32_t i = 0; i < count; ++i) {
        exts.emplace_back(rawExts[i]);

        TBD_DEBUG("GLFW requiring extension \"" << rawExts[i] << "\"");
    }

    return exts;
}

[[nodiscard]] VkSurfaceKHR Window::createVkSurface(VkInstance instance) const
{
    VkSurfaceKHR surface;

    if (glfwCreateWindowSurface(instance, _window, nullptr, &surface) != VK_SUCCESS) {
        ABORT_VK("Failed to create the window surface");
    }

    return surface;
}

} // namespace TBD