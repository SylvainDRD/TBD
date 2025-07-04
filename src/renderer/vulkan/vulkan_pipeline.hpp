#pragma once

#include "vulkan/vulkan_core.h"
#include <filesystem>
#include <misc/utils.hpp>

namespace TBD {

struct PipelineShaderData {
    std::filesystem::path computeShaderPath;
};

class VulkanPipeline {
    TBD_NO_COPY_MOVE(VulkanPipeline)
public:
    VulkanPipeline(VkDevice device, VkDescriptorSetLayout layout, const PipelineShaderData& data);

    [[nodiscard]] bool isValid() const { return _pipeline != nullptr; }

    void release(VkDevice device);

private:
    bool loadShader(VkDevice device, const std::filesystem::path& path, VkShaderModule& module) const;

private:
    VkPipeline _pipeline = nullptr;
    VkPipelineLayout _pipelineLayout;

    std::vector<VkShaderModule> _shaderModules;
};

}