#include "vulkan_buffer.hpp"
#include <vulkan/vulkan_core.h>

namespace TBD {

VulkanBuffer::VulkanBuffer(VmaAllocator allocator, uint32_t size, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage)
{
    VkBufferCreateInfo bufferCreateInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usageFlags
    };

    VmaAllocationCreateInfo allocationCreateInfo {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = memoryUsage
    };

    if (vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &_buffer, &_allocation, &_allocationInfo) != VK_SUCCESS) {
        TBD_ABORT_VK("Vulkan buffer creation failed");
    }
}

}
