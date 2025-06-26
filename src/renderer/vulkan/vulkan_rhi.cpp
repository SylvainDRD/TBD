#include "vulkan_rhi.hpp"
#include <general/window.hpp>
#include <vulkan/vulkan_core.h>
#include "vulkan_utils.hpp"

namespace TBD {

VulkanRHI::VulkanRHI(const Window& Window)
{
    const VkInstanceCreateInfo instanceCreateInfo = initInstanceCreateInfo(Window.requiredVulkanExtensions());
    
    // TODO: fix, the vector holding the extension names is clear on initInstanceCreateInfo return
    // TODO: check result + test layers
    vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);
}

VulkanRHI::~VulkanRHI() {
    vkDestroyInstance(_instance, nullptr);
}

} // namespace TBD