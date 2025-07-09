#pragma once

#include <misc/utils.hpp>
#include <renderer/vulkan/vulkan_rhi.hpp>
#include <vulkan/vulkan_core.h>

namespace TBD {

class VulkanBuffer {
    TBD_NO_COPY(VulkanBuffer)
public:
    VulkanBuffer() = delete;

    VulkanBuffer(VmaAllocator allocator, uint32_t size, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage);

    void release(const IRHI& rhi);

    [[nodiscard]] bool isValid() const { return _buffer != nullptr; }

private:
    VkBuffer _buffer;
    VmaAllocation _allocation = nullptr;
    VmaAllocationInfo _allocationInfo {};
};

}