#include "vulkan_descriptor_set_pool.hpp"

namespace TBD {

VulkanDescriptorSetPool::VulkanDescriptorSetPool(VkDevice device, VkShaderStageFlags stageFlags, std::initializer_list<VkDescriptorType> bindingTypes, uint32_t maxSets)
    : _stageFlags { stageFlags }, _maxSets{maxSets}
{
    _descriptors.reserve(_maxSets);
    _availableSetIDs.reserve(_maxSets);

    // TODO: write a custom linear allocator for these kind of small allocations
    std::vector<VkDescriptorSetLayoutBinding> descriptorBindings { bindingTypes.size() };

    std::unordered_map<VkDescriptorType, uint32_t> descriptorTypeCounts {};
    descriptorTypeCounts.reserve(bindingTypes.size());

    uint32_t i = 0;
    for (VkDescriptorType descriptorType : bindingTypes) {
        descriptorBindings.emplace_back(i++, descriptorType, 1, stageFlags, nullptr);
        ++descriptorTypeCounts[descriptorType];
    }

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = 0,
        .bindingCount = static_cast<uint32_t>(descriptorBindings.size()),
        .pBindings = descriptorBindings.data(),
    };

    if (vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &_layout) != VK_SUCCESS) {
        TBD_ABORT_VK("Failed to create Vulkan descritor set layout");
    }

    std::vector<VkDescriptorPoolSize> poolSizes {};
    poolSizes.reserve(descriptorTypeCounts.size());

    for (auto [descriptorType, count] : descriptorTypeCounts) {
        poolSizes.emplace_back(VkDescriptorPoolSize { descriptorType, _maxSets * count });
    }

    VkDescriptorPoolCreateInfo poolCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = _maxSets,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data()
    };

    if (vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &_pool) != VK_SUCCESS) {
        TBD_ABORT_VK("Failed to create Vulkan descriptor pool");
    }
}

std::pair<uint32_t, VkDescriptorSet> VulkanDescriptorSetPool::getDescriptorSet(VkDevice device)
{
    if (_availableSetIDs.empty()) {
        if (_descriptors.size() == _maxSets) {
            return { TBD_MAX_T(uint32_t), nullptr };
        }

        allocateSet(device);
    }

    uint32_t id = _availableSetIDs.back();
    _availableSetIDs.pop_back();

    return { id, _descriptors[id] };
}


 VkDescriptorSetLayout VulkanDescriptorSetPool::getLayout() const {
    return _layout;
}

void VulkanDescriptorSetPool::releaseSet(uint32_t id)
{
    _availableSetIDs.emplace_back(id);
}

void VulkanDescriptorSetPool::clearPool(VkDevice device)
{
    vkResetDescriptorPool(device, _pool, 0);
    _descriptors.clear();
    _availableSetIDs.clear();
}

void VulkanDescriptorSetPool::releasePool(VkDevice device)
{
    vkDestroyDescriptorSetLayout(device, _layout, nullptr);
    vkDestroyDescriptorPool(device, _pool, nullptr);
}

void VulkanDescriptorSetPool::allocateSet(VkDevice device)
{
    VkDescriptorSetAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = _pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &_layout
    };

    _availableSetIDs.emplace_back(_descriptors.size());
    _descriptors.emplace_back(nullptr);

    if (vkAllocateDescriptorSets(device, &allocInfo, &_descriptors.back()) != VK_SUCCESS) {
        TBD_ABORT_VK("Failed to allocate Vulkan descriptor set");
    }
}

}