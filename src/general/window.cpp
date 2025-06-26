#include "window.hpp"
#include "GLFW/glfw3.h"
#include "misc/utils.hpp"
#include <vector>

namespace TBD {

Window::Window()
{
    if (!glfwInit()) {
        ABORT("GLFW init failed");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _window = glfwCreateWindow(1600, 800, PROJECT_NAME, nullptr, nullptr);

    if (!_window) {
        ABORT("Window creation failed");
    }

    glfwSwapInterval(1);

    TBD_LOG("Window creation complete");
}

Window::~Window()
{
    glfwDestroyWindow(_window);
    glfwTerminate();
}

[[nodiscard]] std::vector<const char*> Window::requiredVulkanExtensions() const
{
    std::vector<const char*> exts{};

    uint32 count;
    const char **rawExts = glfwGetRequiredInstanceExtensions(&count);

    for(int32 i = 0; i <  count; ++i) {
      exts.emplace_back(rawExts[i]);

      TBD_DEBUG("GLFW requiring extension \"" << rawExts[i] << "\"");
    }

    return exts;
}

} // namespace TBD