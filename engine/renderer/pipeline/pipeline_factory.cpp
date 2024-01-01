/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "pipeline_factory.hpp"

#include "utils/log.hpp"

namespace mcvk::Renderer {
    void PipelineFactory::BuildPipelines(const Device &device, const std::vector<GraphicsPipeline *> &pipelines) {
        Utils::Info("Building " + std::to_string(pipelines.size()) + " pipelines");

        std::vector<VkGraphicsPipelineCreateInfo> pipeline_infos(pipelines.size());
        for (size_t i = 0; i < pipeline_infos.size(); i++) {
            pipeline_infos[i] = pipelines[i]->GetCreateInfo();
        }

        std::vector<VkPipeline> vk_pipelines(pipelines.size());

        // batch create pipelines
        if (vkCreateGraphicsPipelines(device.LogicalDevice(), VK_NULL_HANDLE, static_cast<uint32_t>(pipeline_infos.size()),
            pipeline_infos.data(), nullptr, vk_pipelines.data()) != VK_SUCCESS) {
            Utils::Fatal("Failed to create graphics pipeline");
        }

        // store new pipeline objects in each pipeline abstraction
        for (size_t i = 0; i < vk_pipelines.size(); i++) {
            pipelines[i]->_pipeline = vk_pipelines[i];
        }
    }
}
