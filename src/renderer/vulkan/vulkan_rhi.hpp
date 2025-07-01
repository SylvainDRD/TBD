#pragma once

#include "renderer/rendering_dag/rendering_dag.hpp"
#include <array>
#include <cstdint>
#include <misc/utils.hpp>
#include <renderer/resource_allocator.hpp>
#include <renderer/vulkan/vulkan_texture.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace TBD {

class Window;

class VulkanRHI {
    TBD_NO_COPY_MOVE(VulkanRHI)
public:
    using TextureType = VulkanTexture;
    using BufferType = void; // TODO

public:
    VulkanRHI() = delete;

    VulkanRHI(const Window& _window);

    ~VulkanRHI();

    inline VkDevice getVkDevice() { return _device; }

    inline VmaAllocator getAllocator() { return _allocator; }

    inline VkCommandBuffer getCommandBuffer() { return _commandBuffers[_frameId % MaxFramesInFlight]; }

    inline VulkanTexture& getTexture(RID rid) { return _textures.getResource(rid); }

    void render(const RenderingDAG& rdag);

private:
    VkInstance _instance;
#if PROJECT_DEBUG
    VkDebugUtilsMessengerEXT _debugUtilsMessenger;
#endif
    VkPhysicalDevice _gpu;

    VkSurfaceKHR _surface;

    VkDevice _device;
    VmaAllocator _allocator;

    // TODO: refactor that
    ResourceAllocator<VulkanTexture> _textures;

    VkQueue _graphicsQueue;
    VkQueue _presentQueue;

    VkSwapchainKHR _swapchain;
    std::vector<VulkanTexture*> _swapchainTextures;

    VkCommandPool _commandPool;

    static constexpr uint32_t MaxFramesInFlight = 2;
    std::array<VkCommandBuffer, MaxFramesInFlight> _commandBuffers;
    std::array<VkFence, MaxFramesInFlight> _frameFences;
    std::array<VulkanTexture*, MaxFramesInFlight> _renderTargets;

    std::vector<VkSemaphore> _presentSemaphores;
    std::vector<VkSemaphore> _renderSemaphores;

    uint32_t _frameId = 1;
    uint32_t _swapchainImageId;
};

} // namespace TBD