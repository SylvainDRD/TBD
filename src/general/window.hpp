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

    inline uint32_t width() const { return _width; }

    inline uint32_t height() const { return _height; }

    inline void update() { glfwPollEvents(); }

    [[nodiscard]] inline bool windowClosing() const { return glfwWindowShouldClose(_window); }

    [[nodiscard]] std::vector<const char*> requiredVulkanExtensions() const;

private:
    GLFWwindow* _window;

    uint32_t _width;

    uint32_t _height;
};

} // namespace TBD