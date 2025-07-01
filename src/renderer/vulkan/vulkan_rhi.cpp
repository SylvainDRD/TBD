#include "vulkan_rhi.hpp"
#include "vulkan_utils.hpp"
#include <cmath>
#include <cstdint>
#include <general/window.hpp>
#include <misc/utils.hpp>
#include <renderer/vulkan/vulkan_texture.hpp>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace TBD {

VulkanRHI::VulkanRHI(const Window& Window)
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
    auto [swapchain, surfaceFormat] = createSwapchain(_device, _gpu, _surface, queues, { Window.width(), Window.height() });
    _swapchain = swapchain;

    uint32_t swapchainImageCount;
    vkGetSwapchainImagesKHR(_device, _swapchain, &swapchainImageCount, nullptr);

    std::vector<VkImage> swapchainImages;
    swapchainImages.resize(swapchainImageCount);
    _swapchainTextures.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(_device, _swapchain, &swapchainImageCount, swapchainImages.data());

    VulkanTexture::rhi = this;
    for (int i = 0; i < swapchainImageCount; ++i) {
        RID rid = _textures.allocate(swapchainImages[i], surfaceFormat, VkExtent3D { Window.width(), Window.height(), 1 }, VK_IMAGE_ASPECT_COLOR_BIT);
        _swapchainTextures[i] = &_textures.getResource(rid);
    }

    _presentSemaphores.resize(swapchainImages.size());
    _renderSemaphores.resize(swapchainImages.size());

    _commandPool = VKUtils::createCommandPool(_device, queues.GraphicsQueueFamilyID);

    VKUtils::allocateCommandBuffers(_device, _commandPool, MaxFramesInFlight, _commandBuffers.data());

    for (uint32_t i = 0; i < swapchainImages.size(); ++i) {
        _presentSemaphores[i] = VKUtils::createSemaphore(_device);
        _renderSemaphores[i] = VKUtils::createSemaphore(_device);
    }

    for (uint32_t i = 0; i < MaxFramesInFlight; ++i) {
        _frameFences[i] = VKUtils::createFence(_device);
        _renderTargets[i] = &_textures.getResource(
            _textures.allocate(
                VK_FORMAT_R8G8B8A8_UNORM,
                VkExtent3D { Window.width(), Window.height(), 1 },
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT));
    }
}

VulkanRHI::~VulkanRHI()
{
    vkDeviceWaitIdle(_device);

    _textures.clear();

    for (uint32_t i = 0; i < MaxFramesInFlight; ++i) {
        vkDestroyFence(_device, _frameFences[i], nullptr);
    }

    for (uint32_t i = 0; i < _presentSemaphores.size(); ++i) {
        vkDestroySemaphore(_device, _presentSemaphores[i], nullptr);
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

void VulkanRHI::render(const RenderingDAG& rdag)
{
    // rdag.render<VulkanRHI>(this);

    const uint32_t semaphoreId = _frameId % _presentSemaphores.size();
    const uint32_t inFlightFrameId = _frameId % MaxFramesInFlight;

    if (vkWaitForFences(_device, 1, &_frameFences[inFlightFrameId], VK_TRUE, TBD_MAX_T(uint64_t)) != VK_SUCCESS) {
        ABORT_VK("GPU stall detected");
    }
    vkResetFences(_device, 1, &_frameFences[inFlightFrameId]);

    if (vkAcquireNextImageKHR(_device, _swapchain, TBD_MAX_T(uint64_t), _presentSemaphores[semaphoreId], nullptr, &_swapchainImageId) != VK_SUCCESS) {
        ABORT_VK("Failed to acquire next swapchain image");
    }

    VkCommandBuffer commandBuffer = getCommandBuffer();

    VKUtils::beginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VulkanTexture* renderTarget = _renderTargets[inFlightFrameId];
    renderTarget->transitionImage(commandBuffer, VK_IMAGE_LAYOUT_GENERAL);

    renderTarget->clear({ 0.f, 0.f, 0.5f * (std::sin(_frameId / 100.f) + 1.f), 1.f });
    renderTarget->transitionImage(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

    _swapchainTextures[_swapchainImageId]->transitionImage(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    renderTarget->blit(*_swapchainTextures[_swapchainImageId]);

    _swapchainTextures[_swapchainImageId]->transitionImage(commandBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vkEndCommandBuffer(commandBuffer);
    VKUtils::submitCommandBuffer(_graphicsQueue,
        VKUtils::makeSemaphoreSubmitInfo(_presentSemaphores[semaphoreId], VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR),
        VKUtils::makeSemaphoreSubmitInfo(_renderSemaphores[semaphoreId], VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT),
        commandBuffer,
        _frameFences[inFlightFrameId]);

    VkPresentInfoKHR presentInfo {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &_renderSemaphores[semaphoreId],
        .swapchainCount = 1,
        .pSwapchains = &_swapchain,
        .pImageIndices = &_swapchainImageId,
    };
    vkQueuePresentKHR(_presentQueue, &presentInfo);

    ++_frameId;
}

} // namespace TBD