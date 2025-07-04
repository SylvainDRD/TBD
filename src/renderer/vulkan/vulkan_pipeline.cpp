#include "vulkan_pipeline.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <misc/utils.hpp>
#include <vulkan/vulkan_core.h>

namespace TBD {

VulkanPipeline::VulkanPipeline(VkDevice device, VkDescriptorSetLayout layout, const PipelineShaderData& data)
{
    VkPipelineLayoutCreateInfo layoutCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &layout
    };

    if (vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
        TBD_ABORT_VK("Failed to create the pipeline layout");
    }

    if (!data.computeShaderPath.empty()) {
        VkShaderModule module;
        // std::string path = PROJECT_DIR "/src/renderer/shaders/.cache/gradient.comp.spv";
        if (!loadShader(device, data.computeShaderPath, module)) {
            release(device);
        }
        _shaderModules.emplace_back(module);

        std::filesystem::path shaderNamePath = data.computeShaderPath;
        while (shaderNamePath.has_stem()) {
            shaderNamePath = shaderNamePath.stem();
        }

        VkPipelineShaderStageCreateInfo stageCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = _shaderModules[0],
            .pName = shaderNamePath.c_str()
        };

        VkComputePipelineCreateInfo pipelineCreateInfo {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage = stageCreateInfo,
            .layout = _pipelineLayout,
        };

        if (vkCreateComputePipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &_pipeline) != VK_SUCCESS) {
            TBD_ABORT_VK("Failed to create Vulkan compute pipeline");
        }
    }
}

void VulkanPipeline::release(VkDevice device)
{
    if (_pipelineLayout) {
        vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);
    }

    for (VkShaderModule module : _shaderModules) {
        vkDestroyShaderModule(device, module, nullptr);
    }

    if (_pipeline) {
        vkDestroyPipeline(device, _pipeline, nullptr);
    }
}

bool VulkanPipeline::loadShader(VkDevice device, const std::filesystem::path& path, VkShaderModule& module) const
{
    if (!path.has_filename() || !std::filesystem::exists(path)) {
        TBD_WARN("Shader at \"" << path << "\" does not exist");
        return false;
    }

    std::ifstream file{path, std::ios::ate | std::ios::binary};
    if(!file.is_open()) {
        TBD_WARN("Failed to open shader at \"" << path << "\"");
        return false;
    }

    size_t size = file.tellg();
    file.seekg(0);

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    file.close();

    VkShaderModuleCreateInfo moduleCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = reinterpret_cast<uint32_t*>(buffer.data())
    };

    return vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &module) == VK_SUCCESS;
}

}