#pragma once

#include "misc/types.hpp"
#include <cstring>
#include <misc/utils.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace TBD {

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanValidationCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        TBD_WARN_VK(pCallbackData->pMessage);
    } else if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        TBD_ERROR_VK(pCallbackData->pMessage);
    }

    return VK_FALSE;
}

inline VkInstance createVkInstance(std::vector<const char*>&& requiredWindowExtensions)
{
    VkApplicationInfo appInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = PROJECT_NAME,
        .applicationVersion = VK_MAKE_VERSION(PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH),
        .pEngineName = PROJECT_NAME,
        .engineVersion = VK_MAKE_VERSION(PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH),
        .apiVersion = VK_API_VERSION_1_3
    };

    std::vector<const char*> requiredExtensions = std::move(requiredWindowExtensions);
#if PROJECT_DEBUG
    requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
#endif

    VkInstanceCreateInfo vkInstanceCreateInfo {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .pApplicationInfo = &appInfo,
#if PROJECT_DEBUG
        .enabledLayerCount = 1,
        .ppEnabledLayerNames = &validationLayerName,
#endif
        .enabledExtensionCount = static_cast<uint32>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()
    };

    VkInstance instance;

    if (vkCreateInstance(&vkInstanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
        ABORT_VK("Failed to create Vulkan instance");
    }

    TBD_LOG("Vulkan instance successfully created");
    return instance;
}

inline VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance)
{
    PFN_vkCreateDebugUtilsMessengerEXT createDebugMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
        .pfnUserCallback = vulkanValidationCallback
    };

    VkDebugUtilsMessengerEXT debugMessenger;
    if (createDebugMessenger(instance, &messengerCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        ABORT_VK("Failed to create Vulkan debug utils messenger");
    }

    return debugMessenger;
}

} // namespace TBD