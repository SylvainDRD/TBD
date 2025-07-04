#pragma once

#include <cstdint>
#include <misc/types.hpp>
#include <misc/utils.hpp>
#include <renderer/core/rhi_interface.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace TBD {

class VulkanRHI;

class VulkanTexture {
    TBD_NO_COPY(VulkanTexture)
public:
    VulkanTexture() = default;

    VulkanTexture(VulkanRHI* rhi, VkImage image, VkFormat format, VkExtent3D extent, VkImageAspectFlags aspect);

    VulkanTexture(VulkanRHI* rhi, VkFormat format, VkExtent3D extent, VkImageUsageFlags usage, VkImageAspectFlags aspect, bool mipmap = true);

    VulkanTexture(VulkanTexture&& other);

    VulkanTexture& operator=(VulkanTexture&& other);

    ~VulkanTexture();

    void release(const IRHI& rhi);

    [[nodiscard]] inline uint32_t getWidth() const { return _extent.width; }

    [[nodiscard]] inline uint32_t getHeight() const { return _extent.height; }

    [[nodiscard]] inline VkImageView getView() const { return _view; }

    void changeLayout(VkCommandBuffer commandBuffer, VkImageLayout layout, VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED);

    void clear(VkCommandBuffer commandBuffer, Color color);

    void blit(VkCommandBuffer commandBuffer, VulkanTexture& dst);

private:
    VkImage _image;
    VkImageView _view;
    VkFormat _format;
    VkExtent3D _extent;
    VmaAllocation _allocation;
};

}