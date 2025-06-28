#include "vulkan_rhi.hpp"
#include "misc/utils.hpp"
#include "vulkan_utils.hpp"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <general/window.hpp>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>

namespace TBD {

VulkanRHI::VulkanRHI(const Window& Window)
{
    _instance = createVkInstance(Window.requiredVulkanExtensions());

#if PROJECT_DEBUG
    _debugUtilsMessenger = createDebugMessenger(_instance);
#endif

    _surface = Window.createVkSurface(_instance);

    auto [gpu, queues] = selectPhysicalDevice(_instance, _surface);
    _gpu = gpu;

    _device = createLogicalDevice(_gpu, queues);

    vkGetDeviceQueue(_device, queues.GraphicsQueueFamilyID, 0, &_graphicsQueue);
    vkGetDeviceQueue(_device, queues.PresentQueueFamilyID, 0, &_presentQueue);

    // The extent provided should match the surface, hopefully glfw
    _swapchain = createSwapchain(_device, _gpu, _surface, queues, { Window.width(), Window.height() });

    uint32_t swapchainImageCount;
    vkGetSwapchainImagesKHR(_device, _swapchain, &swapchainImageCount, nullptr);

    _swapchainImages.resize(swapchainImageCount);
    _swapchainImageViews.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(_device, _swapchain, &swapchainImageCount, _swapchainImages.data());

    for (uint32_t i = 0; i < swapchainImageCount; ++i) {
        _swapchainImageViews[i] = createImageView(_device, _swapchainImages[i], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);
    }

    _presentSemaphores.resize(_swapchainImages.size());
    _renderSemaphores.resize(_swapchainImages.size());

    _commandPool = createCommandPool(_device, queues.GraphicsQueueFamilyID);

    allocateCommandBuffers(_device, _commandPool, MaxFramesInFlight, _commandBuffers.data());

    for (uint32_t i = 0; i < _swapchainImages.size(); ++i) {
        _presentSemaphores[i] = createSemaphore(_device);
        _renderSemaphores[i] = createSemaphore(_device);
    }

    for (uint32_t i = 0; i < MaxFramesInFlight; ++i) {
        _frameFences[i] = createFence(_device);
    }
}

VulkanRHI::~VulkanRHI()
{
    vkDeviceWaitIdle(_device);
    
    for (uint32_t i = 0; i < MaxFramesInFlight; ++i) {
        vkDestroyFence(_device, _frameFences[i], nullptr);
    }

    for (uint32_t i = 0; i < _swapchainImages.size(); ++i) {
        vkDestroySemaphore(_device, _presentSemaphores[i], nullptr);
        vkDestroySemaphore(_device, _renderSemaphores[i], nullptr);
    }

    vkDestroyCommandPool(_device, _commandPool, nullptr);

    for (uint32_t i = 0; i < _swapchainImageViews.size(); ++i) {
        vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
    }

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

void VulkanRHI::render()
{
    const uint32_t semaphoreId = _frameId % _swapchainImages.size();
    const uint32_t inFlightFrameId = _frameId % MaxFramesInFlight;

    if (vkWaitForFences(_device, 1, &_frameFences[inFlightFrameId], VK_TRUE, TBD_MAX_T(uint64_t)) != VK_SUCCESS) {
        ABORT_VK("GPU stall detected");
    }
    vkResetFences(_device, 1, &_frameFences[inFlightFrameId]);

    if (vkAcquireNextImageKHR(_device, _swapchain, TBD_MAX_T(uint64_t), _presentSemaphores[semaphoreId], nullptr, &_swapchainImageId) != VK_SUCCESS) {
        ABORT_VK("Failed to acquire next swapchain image");
    }

    VkCommandBuffer commandBuffer = _commandBuffers[inFlightFrameId];
    VkImage currentSwapchainImage = _swapchainImages[_swapchainImageId];

    beginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    transitionImage(commandBuffer, currentSwapchainImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    VkClearColorValue clearColor { { 0.f, 0.f, 0.5f * (std::sin(_frameId / 100.f) + 1.f), 1.f } };

    VkImageSubresourceRange imageRange = makeSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

    vkCmdClearColorImage(commandBuffer, currentSwapchainImage, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &imageRange);

    transitionImage(commandBuffer, currentSwapchainImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    vkEndCommandBuffer(commandBuffer);
    submitCommandBuffer(_graphicsQueue,
        makeSemaphoreSubmitInfo(_presentSemaphores[semaphoreId], VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR),
        makeSemaphoreSubmitInfo(_renderSemaphores[semaphoreId], VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT),
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