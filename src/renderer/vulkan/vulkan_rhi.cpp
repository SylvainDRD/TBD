#include "vulkan_rhi.hpp"
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
}

VulkanRHI::~VulkanRHI()
{
#if PROJECT_DEBUG
    PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT"));
    destroyDebugMessenger(_instance, _debugUtilsMessenger, nullptr);
#endif

    vkDestroyInstance(_instance, nullptr);
}

} // namespace TBD