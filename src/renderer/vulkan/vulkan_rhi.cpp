#include "vulkan_rhi.hpp"
#include "misc/utils.hpp"
#include "vulkan_utils.hpp"
#include <cstddef>
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

    vkGetDeviceQueue(_device, queues.GraphicsQueueFamilyID, 0, &_graphicsQueue);
    vkGetDeviceQueue(_device, queues.PresentQueueFamilyID, 0, &_presentQueue);

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

    _presentSemaphores.resize(_swapchainImages.size());
    _renderSemaphores.resize(_swapchainImages.size());

    _commandPool = createCommandPool(_device, queues.GraphicsQueueFamilyID);

    allocateCommandBuffers(_device, _commandPool, MaxFramesInFlight, _commandBuffers.data());
}

VulkanRHI::~VulkanRHI()
{
    vkDestroyCommandPool(_device, _commandPool, nullptr);

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

    TBD_LOG("Vulkan objects cleanup completed");
}

} // namespace TBD