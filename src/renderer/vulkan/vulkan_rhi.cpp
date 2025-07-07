#include "vulkan_rhi.hpp"
#include "vulkan_utils.hpp"
#include <cmath>
#include <cstdint>
#include <general/window.hpp>
#include <misc/utils.hpp>
#include <renderer/vulkan/vulkan_descriptor_set_pool.hpp>
#include <renderer/vulkan/vulkan_pipeline.hpp>
#include <renderer/vulkan/vulkan_texture.hpp>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace TBD {

VulkanRHI::VulkanRHI(const Window& Window)
    : IRHI {}
{
    _instance = VKUtils::createVkInstance(Window.requiredVulkanExtensions());

#if PROJECT_DEBUG
    _debugUtilsMessenger = VKUtils::createDebugMessenger(_instance);
#endif

    _surface = Window.createVkSurface(_instance);

    auto [gpu, queues] = VKUtils::selectPhysicalDevice(_instance, _surface);
    _gpu = gpu;

    _device = createLogicalDevice(_gpu, queues);
    _allocator = VKUtils::createVMAAllocator(_instance, _gpu, _device);

    vkGetDeviceQueue(_device, queues.GraphicsQueueFamilyID, 0, &_graphicsQueue);
    vkGetDeviceQueue(_device, queues.PresentQueueFamilyID, 0, &_presentQueue);

    // The extent provided should match the surface, hopefully glfw
    auto [swapchain, surfaceFormat] = createSwapchain(_device, _gpu, _surface, queues, { Window.getWidth(), Window.getHeight() });
    _swapchain = swapchain;

    uint32_t swapchainImageCount;
    vkGetSwapchainImagesKHR(_device, _swapchain, &swapchainImageCount, nullptr);

    std::vector<VkImage> swapchainImages;
    swapchainImages.resize(swapchainImageCount);
    _swapchainTextures.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(_device, _swapchain, &swapchainImageCount, swapchainImages.data());

    for (int i = 0; i < swapchainImageCount; ++i) {
        RID rid = _textures.allocate(this, swapchainImages[i], surfaceFormat, VkExtent3D { Window.getWidth(), Window.getHeight(), 1 }, VK_IMAGE_ASPECT_COLOR_BIT);
        _swapchainTextures[i] = &_textures.getResource(rid);
    }

    _commandPool = VKUtils::createCommandPool(_device, queues.GraphicsQueueFamilyID);

    VKUtils::allocateCommandBuffers(_device, _commandPool, MaxFramesInFlight, _commandBuffers.data());

    _renderSemaphores.resize(swapchainImageCount);
    for (uint32_t i = 0; i < _renderSemaphores.size(); ++i) {
        _renderSemaphores[i] = VKUtils::createSemaphore(_device);
    }

    for (uint32_t i = 0; i < MaxFramesInFlight; ++i) {
        _presentSemaphores[i] = VKUtils::createSemaphore(_device);
        _frameFences[i] = VKUtils::createFence(_device);
        _renderTargets[i] = &_textures.getResource(
            _textures.allocate(
                this,
                VK_FORMAT_R16G16B16A16_SFLOAT,
                VkExtent3D { Window.getWidth(), Window.getHeight(), 1 },
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT));
    }

    _descriptorSetPool = new VulkanDescriptorSetPool<MaxFramesInFlight>(_device, VK_SHADER_STAGE_COMPUTE_BIT, { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE }, 1000);
    _computePipeline = new VulkanPipeline(
        _device,
        _descriptorSetPool->getLayout(),
        { .computeShaderPath = PROJECT_DIR "src/renderer/shaders/.cache/gradient.comp.spv" });
}

VulkanRHI::~VulkanRHI()
{
    vkDeviceWaitIdle(_device);

    _descriptorSetPool->releasePool(_device);
    delete _descriptorSetPool;

    _computePipeline->release(_device);
    delete _computePipeline;

    _textures.clear(*this);

    for (uint32_t i = 0; i < MaxFramesInFlight; ++i) {
        vkDestroyFence(_device, _frameFences[i], nullptr);
        vkDestroySemaphore(_device, _presentSemaphores[i], nullptr);
    }

    for (uint32_t i = 0; i < _renderSemaphores.size(); ++i) {
        vkDestroySemaphore(_device, _renderSemaphores[i], nullptr);
    }

    vkDestroyCommandPool(_device, _commandPool, nullptr);

    vmaDestroyAllocator(_allocator);

    vkDestroySwapchainKHR(_device, _swapchain, nullptr);
    vkDestroyDevice(_device, nullptr);
    vkDestroySurfaceKHR(_instance, _surface, nullptr);

#if PROJECT_DEBUG
    PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT"));
    destroyDebugMessenger(_instance, _debugUtilsMessenger, nullptr);
#endif

    vkDestroyInstance(_instance, nullptr);

    TBD_LOG("Vulkan objects cleanup completed");
}

void VulkanRHI::render(const RenderingDAG& rdag) const
{
    // rdag.render<VulkanRHI>(this);

    const uint32_t frameInFlightId = _frameId % MaxFramesInFlight;

    if (vkWaitForFences(_device, 1, &_frameFences[frameInFlightId], VK_TRUE, TBD_MAX_T(uint64_t)) != VK_SUCCESS) {
        TBD_ABORT_VK("GPU stall detected");
    }
    vkResetFences(_device, 1, &_frameFences[frameInFlightId]);

    uint32_t swapchainImageId;
    if (vkAcquireNextImageKHR(_device, _swapchain, TBD_MAX_T(uint64_t), _presentSemaphores[frameInFlightId], nullptr, &swapchainImageId) != VK_SUCCESS) {
        TBD_ABORT_VK("Failed to acquire next swapchain image");
    }

    VkCommandBuffer commandBuffer = getCommandBuffer();

    VKUtils::beginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VulkanTexture* renderTarget = _renderTargets[frameInFlightId];
    renderTarget->insertBarrier(commandBuffer, VK_IMAGE_LAYOUT_GENERAL);

    VkDescriptorSet descriptorSet = _descriptorSetPool->getDescriptorSet(_device, frameInFlightId);

    // update DS, bind pipeline, bind DS, dispatch
    VkDescriptorImageInfo imageInfo {
        .imageView = renderTarget->getView(),
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL
    };
    _descriptorSetPool->updateDescriptorSet(_device, commandBuffer, descriptorSet, _computePipeline->getLayout(), imageInfo);
    _computePipeline->bind(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
    _descriptorSetPool->bind(commandBuffer, descriptorSet, VK_PIPELINE_BIND_POINT_COMPUTE, _computePipeline->getLayout());
    _computePipeline->dispatch(commandBuffer, { std::ceil(renderTarget->getWidth() / 8.f), std::ceil(renderTarget->getHeight() / 8.f), 1 });

    renderTarget->insertBarrier(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
    
    _swapchainTextures[swapchainImageId]->insertBarrier(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    renderTarget->blit(commandBuffer, *_swapchainTextures[swapchainImageId]);

    _swapchainTextures[swapchainImageId]->insertBarrier(commandBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vkEndCommandBuffer(commandBuffer);
    VKUtils::submitCommandBuffer(_graphicsQueue,
        VKUtils::makeSemaphoreSubmitInfo(_presentSemaphores[frameInFlightId], VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR),
        VKUtils::makeSemaphoreSubmitInfo(_renderSemaphores[swapchainImageId], VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT),
        commandBuffer,
        _frameFences[frameInFlightId]);

    VkPresentInfoKHR presentInfo {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &_renderSemaphores[swapchainImageId],
        .swapchainCount = 1,
        .pSwapchains = &_swapchain,
        .pImageIndices = &swapchainImageId,
    };
    vkQueuePresentKHR(_presentQueue, &presentInfo);

    ++_frameId;
}

} // namespace TBD