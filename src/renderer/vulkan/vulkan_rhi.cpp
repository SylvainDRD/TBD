#include "vulkan_rhi.hpp"
#include "misc/utils.hpp"
#include "vulkan_utils.hpp"
#include <general/window.hpp>
#include <vulkan/vulkan_core.h>

namespace TBD {

VulkanRHI::VulkanRHI(const Window& Window)
{
    _instance = createVkInstance(Window.requiredVulkanExtensions());
#if PROJECT_DEBUG
    _debugUtilsMessenger = createDebugMessenger(_instance);
#endif

    _surface = Window.createVkSurface(_instance);

    auto [gpu, queues] = selectPhysicalDevice(_instance, _surface);
    _gpu = gpu;

    _device = createLogicalDevice(_gpu, queues);

    vkGetDeviceQueue(_device, queues.GraphicsQueueID, 0, &_graphicsQueue);
    vkGetDeviceQueue(_device, queues.PresentQueueID, 0, &_presentQueue);

    // The extent provided should match the surface, hopefully glfw
    _swapchain = createSwapchain(_device, _gpu, _surface, queues, { Window.width(), Window.height() });

    uint32_t swapchainImageCount;
    vkGetSwapchainImagesKHR(_device, _swapchain, &swapchainImageCount, nullptr);

    _swapchainImages.resize(swapchainImageCount);
    _swapchainImageViews.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(_device, _swapchain, &swapchainImageCount, _swapchainImages.data());

    for (uint32_t i = 0; i < swapchainImageCount; ++i) {
        _swapchainImageViews[i] = createImageView(_device, _swapchainImages[i], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);
    }
}

VulkanRHI::~VulkanRHI()
{
    for (uint32_t i = 0; i < _swapchainImageViews.size(); ++i) {
        vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(_device, _swapchain, nullptr);
    vkDestroyDevice(_device, nullptr);
    vkDestroySurfaceKHR(_instance, _surface, nullptr);

#if PROJECT_DEBUG
    PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT"));
    destroyDebugMessenger(_instance, _debugUtilsMessenger, nullptr);
#endif

    vkDestroyInstance(_instance, nullptr);

    TBD_LOG("Vulkan objects cleanup complete");
}

} // namespace TBD