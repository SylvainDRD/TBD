#pragma once

#include "renderer/vulkan/vulkan_texture.hpp"
#include <array>
#include <cstdint>
#include <misc/utils.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

namespace TBD {

class Window;

class VulkanRHI {
    TBD_NO_COPY_MOVE(VulkanRHI)
public:
    VulkanRHI() = delete;

    VulkanRHI(const Window& _window);

    ~VulkanRHI();

    static inline VkDevice device() { return _device; }

    static inline VmaAllocator allocator() { return _allocator; }

    void render();

private:
    VkInstance _instance;
#if PROJECT_DEBUG
    VkDebugUtilsMessengerEXT _debugUtilsMessenger;
#endif
    VkPhysicalDevice _gpu;
    
    VkSurfaceKHR _surface;

    static VkDevice _device;
    static VmaAllocator _allocator;

    VkQueue _graphicsQueue;
    VkQueue _presentQueue;

    VkSwapchainKHR _swapchain;
    std::vector<VkImage> _swapchainImages;
    std::vector<VkImageView> _swapchainImageViews;

    VkCommandPool _commandPool;

    static constexpr uint32_t MaxFramesInFlight = 2;
    std::array<VkCommandBuffer, MaxFramesInFlight> _commandBuffers;
    std::array<VkFence, MaxFramesInFlight> _frameFences;
    std::array<VulkanTexture, MaxFramesInFlight> _renderTargets;

    // 1 per swapchain image, basing this off Sascha Willems VK samples: https://github.com/SaschaWillems/Vulkan
    // Not convinced that this is necessary
    std::vector<VkSemaphore> _presentSemaphores;
    std::vector<VkSemaphore> _renderSemaphores;

    uint32_t _frameId = 1;
    uint32_t _swapchainImageId;
};

} // namespace TBD