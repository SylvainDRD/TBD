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
    requiredExtensions.emplace_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    requiredExtensions.emplace_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    requiredExtensions.emplace_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
#if PROJECT_DEBUG
    requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
#endif

    // TODO: delete
    uint32_t extCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);

    std::vector<VkExtensionProperties> exts{extCount};
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, exts.data());

    for(const auto& ext: exts) {
        TBD_DEBUG(ext.extensionName);
    }

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

    inline bool isValid() { return GraphicsQueueID != TBD_MAX_T(uint32_t) && PresentQueueID != TBD_MAX_T(uint32_t); }
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
    std::vector<VkDeviceQueueCreateInfo> createInfos {};
    VkDeviceQueueCreateInfo queueCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueCount = 1,
        .pQueuePriorities = &priority
    };
    for (uint32_t queueID : queueIndices) {
        queueCreateInfo.queueFamilyIndex = queueID;

        createInfos.emplace_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures features {};
    VkDeviceCreateInfo deviceCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(createInfos.size()),
        .pQueueCreateInfos = createInfos.data(),
        .pEnabledFeatures = &features
    };

    VkDevice device;
    if (vkCreateDevice(gpu, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
        ABORT_VK("Vulkan device creation failed");
    }

    TBD_LOG("Vulkan device created successfully");

    return device;
}

} // namespace TBD