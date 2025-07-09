#pragma once

#include "misc/types.hpp"
#include "vulkan/vulkan_core.h"
#include <filesystem>
#include <misc/utils.hpp>

namespace TBD {

struct PipelineShaderData {
    std::filesystem::path computeShaderPath;
    std::filesystem::path vertexShaderPath;
    std::filesystem::path fragmentShaderPath;
    std::vector<VkFormat> colorAttachmentFormats;
    VkFormat depthAttachmentFormat;
    VkFormat stencilAttachmentFormat;
};

class VulkanPipeline {
    TBD_NO_COPY_MOVE(VulkanPipeline)
public:
    VulkanPipeline(VkDevice device, VkDescriptorSetLayout layout, const PipelineShaderData& data);

    void dispatch(VkCommandBuffer commandBuffer, Vec3i kernelSize);

    void draw(VkCommandBuffer commandBuffer, VkExtent2D extent, const std::vector<VkRenderingAttachmentInfo>& colorAttachments, VkRenderingAttachmentInfo depthAttachment = {}, VkRenderingAttachmentInfo stencilAttachment = {});

    [[nodiscard]]
    VkPipelineLayout getLayout() const
    {
        return _pipelineLayout;
    }

    [[nodiscard]]
    bool isValid() const
    {
        return _pipeline != nullptr;
    }

    void release(VkDevice device);

private:
    bool loadShader(VkDevice device, const std::filesystem::path& path, VkShaderModule& module) const;

private:
    VkPipeline _pipeline = nullptr;
    VkPipelineLayout _pipelineLayout;

    std::vector<VkShaderModule> _shaderModules;
};

}