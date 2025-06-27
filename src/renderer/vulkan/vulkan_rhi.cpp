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

    
}

VulkanRHI::~VulkanRHI()
{
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