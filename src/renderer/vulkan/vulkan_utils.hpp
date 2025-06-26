#pragma once

#include <misc/utils.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace TBD {

inline VkInstanceCreateInfo initInstanceCreateInfo(std::vector<const char*>&& requiredWindowExtensions)
{
    VkApplicationInfo appInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = PROJECT_NAME,
        .applicationVersion = VK_MAKE_VERSION(PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH),
        .pEngineName = PROJECT_NAME,
        .engineVersion = VK_MAKE_VERSION(PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH),
        .apiVersion = VK_API_VERSION_1_3
    };

    const char* validationLayerName = "VK_LAYER_KHRONOS_validation";

    std::vector<const char*> requiredExtensions = std::move(requiredWindowExtensions);

    VkInstanceCreateInfo vkInstanceCreateInfo {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .pApplicationInfo = &appInfo,
#if !PROJECT_DEBUG
        .enabledLayerCount = 1,
        .ppEnabledLayerNames = &validationLayerName,
#endif
        .enabledExtensionCount = static_cast<uint32>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()
    };

    return vkInstanceCreateInfo;
}

} // namespace TBD