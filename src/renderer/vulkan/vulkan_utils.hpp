#pragma once

#include <cstdint>
#include <cstring>
#include <limits>
#include <misc/utils.hpp>
#include <sys/types.h>
#include <unordered_set>
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
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
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

    TBD_LOG("Vulkan debug messenger successfully created");
    return debugMessenger;
}

struct PhysicalDeviceQueueFamilyID {
    uint32_t GraphicsQueueID = TBD_MAX_T(uint32_t);
    uint32_t PresentQueueID = TBD_MAX_T(uint32_t);

    inline bool isValid() const { return GraphicsQueueID != TBD_MAX_T(uint32_t) && PresentQueueID != TBD_MAX_T(uint32_t); }
};

inline std::pair<VkPhysicalDevice, PhysicalDeviceQueueFamilyID> selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

    std::vector<VkPhysicalDevice> availableGpus { physicalDeviceCount };
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, availableGpus.data());

    PhysicalDeviceQueueFamilyID queues {};
    std::vector<VkQueueFamilyProperties> queueData { 16 };

    const char* selectedDeviceName;
    uint32_t deviceId = TBD_MAX_T(uint32_t);

    for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(availableGpus[i], &properties);

        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
            || properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU && deviceId == TBD_MAX_T(uint32_t)) {
            uint32_t familyCount;
            vkGetPhysicalDeviceQueueFamilyProperties(availableGpus[i], &familyCount, nullptr);

            queueData.resize(familyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(availableGpus[i], &familyCount, queueData.data());

            queues = {};
            for (int32_t queueId = 0; queueId < familyCount; ++queueId) {
                if (queues.GraphicsQueueID == TBD_MAX_T(decltype(queues.GraphicsQueueID)) && queueData[queueId].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    queues.GraphicsQueueID = queueId;
                }

                if (queues.PresentQueueID == TBD_MAX_T(decltype(queues.PresentQueueID))) {
                    VkBool32 supported;
                    vkGetPhysicalDeviceSurfaceSupportKHR(availableGpus[i], queueId, surface, &supported);
                    queues.PresentQueueID = queueId;
                }
            }

            deviceId = i;
            selectedDeviceName = properties.deviceName;
        }
    }

    if (deviceId == TBD_MAX_T(uint32_t)) {
        ABORT_VK("Couldn't find a suitable physical device");
    }

    VkPhysicalDevice gpu = availableGpus[deviceId];

    TBD_LOG("Selected Vulkan device: " << selectedDeviceName);

    return { availableGpus[deviceId], queues };
}

inline VkDevice createLogicalDevice(VkPhysicalDevice gpu, PhysicalDeviceQueueFamilyID queues)
{
    std::unordered_set<uint32_t> queueIndices { queues.GraphicsQueueID, queues.PresentQueueID };

    const float priority = 1.f;
    std::vector<VkDeviceQueueCreateInfo> queuesCreateInfo {};
    VkDeviceQueueCreateInfo queueCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueCount = 1,
        .pQueuePriorities = &priority
    };
    for (uint32_t queueID : queueIndices) {
        queueCreateInfo.queueFamilyIndex = queueID;

        queuesCreateInfo.emplace_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures features {};
    VkPhysicalDeviceVulkan12Features features12 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = VK_TRUE,
        .bufferDeviceAddress = VK_TRUE
    };
    VkPhysicalDeviceVulkan13Features features13 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &features12,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE
    };

    const char* swapchainExt = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

    VkDeviceCreateInfo deviceCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features13,
        .queueCreateInfoCount = static_cast<uint32_t>(queuesCreateInfo.size()),
        .pQueueCreateInfos = queuesCreateInfo.data(),
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = &swapchainExt,
        .pEnabledFeatures = &features
    };

    VkDevice device;
    if (vkCreateDevice(gpu, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
        ABORT_VK("Vulkan device creation failed");
    }

    TBD_LOG("Vulkan device created successfully");

    return device;
}

inline VkSwapchainKHR createSwapchain(VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface, PhysicalDeviceQueueFamilyID queues, VkExtent2D extent, VkSwapchainKHR previousSwapchain = nullptr)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfaceCapabilities);

    VkSwapchainCreateInfoKHR swapchainCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = surfaceCapabilities.maxImageCount > 0 && surfaceCapabilities.minImageCount + 1 > surfaceCapabilities.maxImageCount ? surfaceCapabilities.maxImageCount : surfaceCapabilities.minImageCount + 1,
        .imageFormat = VK_FORMAT_R8G8B8A8_SRGB,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = previousSwapchain
    };

    if (queues.GraphicsQueueID != queues.PresentQueueID) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = (uint32_t*)&queues; // a bit freaky
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    VkSwapchainKHR swapchain;

    if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS) {
        ABORT_VK("Vulkan swapchain creation failed");
    }

    return swapchain;
}

inline VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType viewType)
{
    VkImageViewCreateInfo viewCreateInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = viewType,
        .format = format,
        .components = {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A },
        .subresourceRange = { .aspectMask = aspectFlags, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 }
    };

    VkImageView view;
    if (vkCreateImageView(device, &viewCreateInfo, nullptr, &view) != VK_SUCCESS) {
        ABORT_VK("Vulkan image view creation failed");
    }

    return view;
}

} // namespace TBD