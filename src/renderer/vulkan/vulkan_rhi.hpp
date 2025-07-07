#pragma once

#include <array>
#include <cstdint>
#include <misc/utils.hpp>
#include <renderer/core/resource_allocator.hpp>
#include <renderer/core/rhi_interface.hpp>
#include <renderer/rendering_dag/rendering_dag.hpp>
#include <renderer/vulkan/vulkan_texture.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace TBD {

class Window;
template<uint32_t>
class VulkanDescriptorSetPool;
class VulkanPipeline;

class VulkanRHI : public IRHI {
    TBD_NO_COPY_MOVE(VulkanRHI)
public:
    using Type = VulkanRHI;
    using TextureType = VulkanTexture;
    using BufferType = void; // TODO

public:
    VulkanRHI() = delete;

    VulkanRHI(const Window& _window);

    ~VulkanRHI();

    inline VkDevice getVkDevice() const { return _device; }

    inline VmaAllocator getAllocator() const { return _allocator; }

    inline VkCommandBuffer getCommandBuffer() const { return _commandBuffers[_frameId % MaxFramesInFlight]; }

    inline VulkanTexture& getTexture(RID rid) { return _textures.getResource(rid); }

    virtual void render(const RenderingDAG& rdag) const override;

private:
    VkInstance _instance;
#if PROJECT_DEBUG
    VkDebugUtilsMessengerEXT _debugUtilsMessenger;
#endif
    VkPhysicalDevice _gpu;

    VkSurfaceKHR _surface;

    VkDevice _device;
    VmaAllocator _allocator;

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

    // TODO: refactor that
    ResourceAllocator<VulkanTexture> _textures;
    VulkanDescriptorSetPool<MaxFramesInFlight>* _descriptorSetPool;
    VulkanPipeline* _computePipeline;

    mutable uint32_t _frameId = 1;
};

} // namespace TBD