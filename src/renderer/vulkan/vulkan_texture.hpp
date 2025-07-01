#pragma once

#include "misc/types.hpp"
#include <cstdint>
#include <misc/utils.hpp>
#include <renderer/resource_allocator.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace TBD {

class VulkanRHI;

class VulkanTexture {
    TBD_NO_COPY(VulkanTexture)
public:
    VulkanTexture() = default;

    VulkanTexture(VkImage image, VkFormat format, VkExtent3D extent, VkImageAspectFlags aspect);

    VulkanTexture(VkFormat format, VkExtent3D extent, VkImageUsageFlags usage, VkImageAspectFlags aspect, bool mipmap = true);

    VulkanTexture(VulkanTexture&& other);

    VulkanTexture& operator=(VulkanTexture&& other);

    ~VulkanTexture();

    [[nodiscard]] inline uint32_t width() { return _extent.width; }

    [[nodiscard]] inline uint32_t height() { return _extent.height; }

    void transitionImage(VkCommandBuffer commandBuffer, VkImageLayout layout, VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED);

    void clear(Color color);

    void blit(VulkanTexture& dst);

public:
    static VulkanRHI* rhi;

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