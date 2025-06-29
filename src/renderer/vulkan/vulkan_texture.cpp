#include "vulkan_texture.hpp"
#include "renderer/vulkan/vulkan_utils.hpp"
#include <misc/utils.hpp>
#include <renderer/vulkan/vulkan_rhi.hpp>
#include <vulkan/vulkan_core.h>

namespace TBD {

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

    if (vmaCreateImage(VulkanRHI::allocator(), &imageCreateInfo, &allocCreateInfo, &_image, &_allocation, nullptr) != VK_SUCCESS) {
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

    if (vkCreateImageView(VulkanRHI::device(), &viewCreateInfo, nullptr, &_view) != VK_SUCCESS) {
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
        vkDestroyImageView(VulkanRHI::device(), _view, nullptr);
    }

    if (_image) {
        vmaDestroyImage(VulkanRHI::allocator(), _image, _allocation);
    }
}

void VulkanTexture::transitionImage(VkCommandBuffer commandBuffer, VkImageLayout newLayout, VkImageLayout oldLayout)
{
    VKUtils::transitionImage(commandBuffer, _image, oldLayout, newLayout);
}
}