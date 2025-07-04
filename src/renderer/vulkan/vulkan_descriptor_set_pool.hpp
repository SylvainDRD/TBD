#pragma once

#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <initializer_list>
#include <misc/utils.hpp>

namespace TBD {

class VulkanRHI;

class VulkanDescriptorSetPool {
    TBD_NO_COPY_MOVE(VulkanDescriptorSetPool)
public:
    VulkanDescriptorSetPool() = delete;

    // Bindings are expected in order i.e. first descritor type for index 0, second for index 1, etc.
    VulkanDescriptorSetPool(VkDevice device, VkShaderStageFlags stageFlags, std::initializer_list<VkDescriptorType> bindingTypes, uint32_t maxSets);

    [[nodiscard]] std::pair<uint32_t, VkDescriptorSet> getDescriptorSet(VkDevice device);

    [[nodiscard]] VkDescriptorSetLayout getLayout() const;

    void releaseSet(uint32_t id);

    void clearPool(VkDevice device);

    void releasePool(VkDevice device);

private:
    void allocateSet(VkDevice device);

private:
    VkDescriptorSetLayout _layout;

    VkShaderStageFlags _stageFlags;

    VkDescriptorPool _pool;

    std::vector<VkDescriptorSet> _descriptors;

    std::vector<uint32_t> _availableSetIDs;

    uint32_t _maxSets;
};

}