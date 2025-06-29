#pragma once

#include <cstdint>
#include <misc/utils.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace TBD {

class VulkanTexture {
    TBD_NO_COPY(VulkanTexture)
public:
    VulkanTexture() = default;

    VulkanTexture(VkFormat format, VkExtent3D extent, VkImageUsageFlags usage, VkImageAspectFlags aspect, bool mipmap = true);

    VulkanTexture(VulkanTexture&& other);

    VulkanTexture& operator=(VulkanTexture&& other);

    ~VulkanTexture();

    [[nodiscard]] inline VkImage image() { return _image; }

    [[nodiscard]] inline uint32_t width() { return _extent.width; }

    [[nodiscard]] inline uint32_t height() { return _extent.height; }

    void transitionImage(VkCommandBuffer commandBuffer, VkImageLayout layout, VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED);

private:
    void cleanup();

private:
    VkImage _image;
    VkImageView _view;
    VkFormat _format;
    VkExtent3D _extent;
    VmaAllocation _allocation;
};

}