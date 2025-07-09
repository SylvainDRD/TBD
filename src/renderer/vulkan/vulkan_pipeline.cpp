#include "vulkan_pipeline.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <misc/utils.hpp>
#include <vulkan/vulkan_core.h>

namespace TBD {

VulkanPipeline::VulkanPipeline(VkDevice device, VkDescriptorSetLayout layout, const PipelineShaderData& shaderData)
{
    VkPipelineLayoutCreateInfo layoutCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = layout != nullptr ? 1u : 0u,
        .pSetLayouts = &layout
    };

    if (vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
        TBD_ABORT_VK("Failed to create the pipeline layout");
    }

    if (shaderData.computeShaderPath.empty() && (shaderData.vertexShaderPath.empty() || shaderData.fragmentShaderPath.empty())) {
        TBD_ABORT_VK("mandatory vulkan shader path missing");
    } else if (!shaderData.computeShaderPath.empty()) {
        VkShaderModule module;
        if (!loadShader(device, shaderData.computeShaderPath, module)) {
            release(device);
        }
        _shaderModules.emplace_back(module);

        VkPipelineShaderStageCreateInfo stageCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = _shaderModules[0],
            .pName = "main"
        };

        VkComputePipelineCreateInfo pipelineCreateInfo {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage = stageCreateInfo,
            .layout = _pipelineLayout,
        };

        if (vkCreateComputePipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &_pipeline) != VK_SUCCESS) {
            TBD_ABORT_VK("Failed to create Vulkan compute pipeline");
        }
    } else {
        VkPipelineVertexInputStateCreateInfo vertexCI { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

        VkPipelineInputAssemblyStateCreateInfo iasCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
        };

        VkPipelineRasterizationStateCreateInfo rasterCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .lineWidth = 1.f
        };

        VkPipelineViewportStateCreateInfo viewportCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1
        };

        VkPipelineMultisampleStateCreateInfo msaaCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.f,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };

        VkPipelineDepthStencilStateCreateInfo depthStencilCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0.f,
            .maxDepthBounds = 1.f
        };

        VkPipelineColorBlendAttachmentState blendAttachment {
            .blendEnable = VK_FALSE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineColorBlendStateCreateInfo blendCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE, // TODO: hmmmm
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &blendAttachment
        };

        VkDynamicState dynState[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynStateCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = sizeof(dynState) / sizeof(VkDynamicState),
            .pDynamicStates = dynState
        };

        VkShaderModule vertexShader;
        if (!loadShader(device, shaderData.vertexShaderPath, vertexShader)) {
            release(device);
        }
        _shaderModules.emplace_back(vertexShader);

        VkShaderModule fragmentShader;
        if (!loadShader(device, shaderData.fragmentShaderPath, fragmentShader)) {
            release(device);
        }
        _shaderModules.emplace_back(fragmentShader);

        VkPipelineShaderStageCreateInfo vertexStageCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShader,
            .pName = "main"
        };

        VkPipelineShaderStageCreateInfo fragmentStageCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShader,
            .pName = "main"
        };

        const VkPipelineShaderStageCreateInfo stagesCI[] = {
            vertexStageCI,
            fragmentStageCI
        };

        VkPipelineRenderingCreateInfo renderCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = static_cast<uint32_t>(shaderData.colorAttachmentFormats.size()),
            .pColorAttachmentFormats = shaderData.colorAttachmentFormats.data(),
            .depthAttachmentFormat = shaderData.depthAttachmentFormat,
            .stencilAttachmentFormat = shaderData.stencilAttachmentFormat
        };

        VkGraphicsPipelineCreateInfo pipelineCreateInfo {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &renderCI,
            .stageCount = 2, // TODO: expand to support more than fragment and vertex shaders
            .pStages = stagesCI,
            .pVertexInputState = &vertexCI,
            .pInputAssemblyState = &iasCI,
            .pViewportState = &viewportCI,
            .pRasterizationState = &rasterCI,
            .pMultisampleState = &msaaCI,
            .pDepthStencilState = &depthStencilCI,
            .pColorBlendState = &blendCI,
            .pDynamicState = &dynStateCI,
            .layout = _pipelineLayout
        };

        if (vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &_pipeline) != VK_SUCCESS) {
            TBD_ABORT_VK("Failed to create graphics pipeline");
        }
    }
}

void VulkanPipeline::dispatch(VkCommandBuffer commandBuffer, Vec3i kernelSize)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline);

    vkCmdDispatch(commandBuffer, kernelSize.x, kernelSize.y, kernelSize.z);
}

void VulkanPipeline::draw(VkCommandBuffer commandBuffer, VkExtent2D extent, const std::vector<VkRenderingAttachmentInfo>& attachments, VkRenderingAttachmentInfo depthAttachment, VkRenderingAttachmentInfo stencilAttachment)
{
    VkRenderingInfo renderingInfo {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = VkRect2D { {}, extent },
        .layerCount = 1,
        .colorAttachmentCount = static_cast<uint32_t>(attachments.size()),
        .pColorAttachments = attachments.data(),
        .pDepthAttachment = depthAttachment.imageView != nullptr ? &depthAttachment : nullptr,
        .pStencilAttachment = stencilAttachment.imageView != nullptr ? &stencilAttachment : nullptr,
    };

    vkCmdBeginRendering(commandBuffer, &renderingInfo);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

    VkViewport viewport = {
        .x = 0,
        .y = 0,
        .width = float(extent.width),
        .height = float(extent.height),
        .minDepth = 0.f,
        .maxDepth = 1.f
    };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .extent = extent,
    };

    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRendering(commandBuffer);
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

    std::ifstream file { path, std::ios::ate | std::ios::binary };
    if (!file.is_open()) {
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