#include "vulkan_texture.hpp"
#include <misc/utils.hpp>
#include <renderer/vulkan/vulkan_rhi.hpp>
#include <renderer/vulkan/vulkan_utils.hpp>
#include <vulkan/vulkan_core.h>

namespace TBD {

VulkanRHI* VulkanTexture::rhi;

VulkanTexture::VulkanTexture(VkImage image, VkFormat format, VkExtent3D extent, VkImageAspectFlags aspect)
    : _image { image }
    , _format { format }
    , _extent { extent }
    , _allocation { nullptr }
{
    VkImageViewCreateInfo viewCreateInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = _image,
        .viewType = extent.depth == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_3D,
        .format = format,
        .subresourceRange = {
            .aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1 }
    };

    if (vkCreateImageView(rhi->getVkDevice(), &viewCreateInfo, nullptr, &_view) != VK_SUCCESS) {
        ABORT_VK("Vulkan image view creation failed");
    }
}

VulkanTexture::VulkanTexture(VkFormat format, VkExtent3D extent, VkImageUsageFlags usage, VkImageAspectFlags aspect, bool mipmap)
    : _format { format }
    , _extent { extent }
{
    VkImageCreateInfo imageCreateInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = extent.depth == 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D,
        .format = format,
        .extent = extent,
        .mipLevels = 1, // TODO
        .arrayLayers = 1, // TODO: use that for 3D textures?
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VmaAllocationCreateInfo allocCreateInfo {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = VkMemoryPropertyFlags { VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT }
    };

    if (vmaCreateImage(rhi->getAllocator(), &imageCreateInfo, &allocCreateInfo, &_image, &_allocation, nullptr) != VK_SUCCESS) {
        ABORT_VK("VMA image creation failed");
    }

    VkImageViewCreateInfo viewCreateInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = _image,
        .viewType = extent.depth == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_3D,
        .format = format,
        .subresourceRange = {
            .aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1 }
    };

    if (vkCreateImageView(rhi->getVkDevice(), &viewCreateInfo, nullptr, &_view) != VK_SUCCESS) {
        ABORT_VK("Failed to create Vulkan image view");
    }
}

VulkanTexture::VulkanTexture(VulkanTexture&& other)
{
    _image = other._image;
    other._image = nullptr;
    _view = other._view;
    other._view = nullptr;
    _allocation = other._allocation;
    other._allocation = nullptr;
    _format = other._format;
    _extent = other._extent;
}

VulkanTexture& VulkanTexture::operator=(VulkanTexture&& other)
{
    // Overwritten texture needs to be cleaned up
    cleanup();

    _image = other._image;
    other._image = nullptr;
    _view = other._view;
    other._view = nullptr;
    _allocation = other._allocation;
    other._allocation = nullptr;
    _format = other._format;
    _extent = other._extent;

    return *this;
}

VulkanTexture::~VulkanTexture()
{
    cleanup();
}

void VulkanTexture::cleanup()
{
    if (_view) {
        vkDestroyImageView(rhi->getVkDevice(), _view, nullptr);
    }

    if (_allocation == nullptr) {
        // Likely a swapchain image
        return;
    }

    if (_image) {
        vmaDestroyImage(rhi->getAllocator(), _image, _allocation);
    }
}

void VulkanTexture::transitionImage(VkCommandBuffer commandBuffer, VkImageLayout newLayout, VkImageLayout oldLayout)
{
    VKUtils::transitionImage(commandBuffer, _image, oldLayout, newLayout);
}

void VulkanTexture::clear(Color color)
{
    VkImageSubresourceRange imageRange = VKUtils::makeSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
    vkCmdClearColorImage(rhi->getCommandBuffer(), _image, VK_IMAGE_LAYOUT_GENERAL, reinterpret_cast<VkClearColorValue*>(&color), 1, &imageRange);
}

void VulkanTexture::blit(VulkanTexture& dst)
{
    VkImageBlit imageBlit {
        .srcSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1 },
        .srcOffsets = { { 0, 0, 0 }, { static_cast<int32_t>(width()), static_cast<int32_t>(height()), 1 } },
        .dstSubresource = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 },
        .dstOffsets = { { 0, 0, 0 }, { static_cast<int32_t>(dst.width()), static_cast<int32_t>(dst.height()), 1 } },
    };

    vkCmdBlitImage(rhi->getCommandBuffer(),
        _image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dst._image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &imageBlit,
        VK_FILTER_NEAREST);
}

}