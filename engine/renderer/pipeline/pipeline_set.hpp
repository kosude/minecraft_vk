/*
 * Copyright (c) 2024 Jack Bennett.
 * All Rights Reserved.
 *
 * See the LICENCE file for more information.
 */

#pragma once

#include "renderer/pipeline/graphics_pipeline.hpp"
#include "renderer/device.hpp"
#include "renderer/swapchain.hpp"

#include "resource_mgr/resource_mgr.hpp"

#include <memory>
#include <unordered_map>

namespace mcvk::Renderer {
    struct PipelineSet {
    public:
        PipelineSet(const Device &device, const std::unique_ptr<Swapchain> &swapchain, const ResourceMgr::ResourceManager &resmgr);

        inline const GraphicsPipeline &GraphicsByName(const std::string &name) const { return *(_graphics_pipelines.at(name)); }

    private:
        friend class Renderer;

        void _Initialise(const std::vector<VkDescriptorSetLayout> &set_layouts);
        void _CreateGraphicsPipelines(const std::vector<VkDescriptorSetLayout> &set_layouts);

        const Device &_device;
        const std::unique_ptr<Swapchain> &_swapchain;
        const ResourceMgr::ResourceManager &_resmgr;

        typedef std::unique_ptr<GraphicsPipeline> GraphicsPipelinePtr;
        std::unordered_map<std::string, GraphicsPipelinePtr> _graphics_pipelines;
    };
}
