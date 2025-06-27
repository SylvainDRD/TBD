#pragma once

#include <misc/utils.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace TBD {

class Window;

class VulkanRHI {
    TBD_NO_COPY_MOVE(VulkanRHI)
public:
    VulkanRHI() = delete;

    VulkanRHI(const Window& _window);

    ~VulkanRHI();

private:
    VkInstance _instance;
#if PROJECT_DEBUG
    VkDebugUtilsMessengerEXT _debugUtilsMessenger;
#endif
    VkPhysicalDevice _gpu;
    
    VkSurfaceKHR _surface;
    VkDevice _device;

    VkQueue _graphicsQueue;
    VkQueue _presentQueue;

    VkSwapchainKHR _swapchain;
    std::vector<VkImage> _swapchainImages;
    std::vector<VkImageView> _swapchainImageViews;
};

} // namespace TBD