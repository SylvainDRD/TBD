#pragma once

#include <misc/utils.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace TBD {

class Window {
    TBD_NO_COPY_MOVE(Window)
public:
    Window();

    ~Window();

    inline void pollEvents() { glfwPollEvents(); }

    [[nodiscard]] inline bool windowClosing() const { return glfwWindowShouldClose(_window); }

    [[nodiscard]] std::vector<const char*> requiredVulkanExtensions() const;

private:
    GLFWwindow* _window;
};

} // namespace TBD