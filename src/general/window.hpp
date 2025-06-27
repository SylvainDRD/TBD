#pragma once

#include <misc/utils.hpp>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

namespace TBD {

class Window {
    TBD_NO_COPY_MOVE(Window)
public:
    Window();

    ~Window();

    [[nodiscard]] VkSurfaceKHR createVkSurface(VkInstance instance) const;

    inline void pollEvents() { glfwPollEvents(); }

    [[nodiscard]] inline bool windowClosing() const { return glfwWindowShouldClose(_window); }

    [[nodiscard]] std::vector<const char*> requiredVulkanExtensions() const;

private:
    GLFWwindow* _window;
};

} // namespace TBD