/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "pipeline_set.hpp"

#include "renderer/pipeline/pipeline.hpp"

#include "utils/log.hpp"

namespace mcvk::Renderer {
    PipelineSet::PipelineSet(const Device &device, const std::unique_ptr<Swapchain> &swapchain, const ResourceMgr::ResourceManager &resmgr)
        : _device{device}, _swapchain{swapchain}, _resmgr{resmgr} {
    }

    void PipelineSet::_Initialise(const std::vector<VkDescriptorSetLayout> &set_layouts) {
        _CreateGraphicsPipelines(set_layouts);
    }

    void PipelineSet::_CreateGraphicsPipelines(const std::vector<VkDescriptorSetLayout> &set_layouts) {
        // find and parse pipeline config resources
        std::vector<ResourceMgr::PipelineResource> pipeline_resources;
        uint32_t i = 0;
        for (const auto &confname : ResourceMgr::ResourceManager::GetAllFilenamesInDir(_resmgr.GetPipelineResourcesDir())) {
            ResourceMgr::PipelineResource res{};
            if (!_resmgr.Load(confname, res)) {
                Utils::Warn("Found pipeline config with filename " + confname + " but failed to parse it. Skipping...");
                continue;
            }
            pipeline_resources.push_back(res);
        }

        auto graphics_config = GraphicsPipeline::Config::Defaults();
        graphics_config.render_pass = _swapchain->GetRenderPass();
        graphics_config.set_layouts = set_layouts;

        for (const auto &res : pipeline_resources) {
            ResourceMgr::ShaderResource shader;
            if (!_resmgr.Load(res.shader_name, shader)) {
                Utils::Warn("Failed to load shader " + res.shader_name + " for pipeline " + res.name + ". Skipping...");
                continue;
            }

            if (res.type == ResourceMgr::PipelineResource::Type::Graphics) {
                graphics_config.rasterization_info.polygonMode = res.polygon_mode;
                graphics_config.rasterization_info.cullMode = res.cull_mode;

                _graphics_pipelines.emplace(res.name, std::make_unique<GraphicsPipeline>(_device, shader.shaders, graphics_config));
            }
        }

        // construct vector of 'unsafe' pipeline pointers (i know this is stupid but if I change any of the types I just get errors so oh well!!!)
        std::vector<GraphicsPipeline *> graphics_pipeline_ptrs(_graphics_pipelines.size());
        i = 0;
        for (const auto &p : _graphics_pipelines) {
            graphics_pipeline_ptrs[i++] = &*(p.second);
        };
        GraphicsPipeline::BuildGraphicsPipelines(_device, graphics_pipeline_ptrs);
    }
}
