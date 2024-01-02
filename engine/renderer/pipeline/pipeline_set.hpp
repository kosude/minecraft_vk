/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/pipeline/graphics_pipeline.hpp"
#include "renderer/device.hpp"
#include "renderer/swapchain.hpp"

#include <memory>

namespace mcvk::Renderer {
    struct PipelineSet {
    public:
        PipelineSet(const Device &device, const std::unique_ptr<Swapchain> &swapchain);

        inline const GraphicsPipeline &SimpleGraphics() const { return *_g_simple; }
        inline const GraphicsPipeline &SimpleWireframeGraphics() const { return *_g_simple_wireframe; }

    private:
        friend class Renderer;

        void _Initialise();
        void _CreateGraphicsPipelines();

        std::vector<ShaderInfo> _GetShaders();

        const Device &_device;
        const std::unique_ptr<Swapchain> &_swapchain;

        typedef std::unique_ptr<GraphicsPipeline> GraphicsPipelinePtr;
        GraphicsPipelinePtr _g_simple;
        GraphicsPipelinePtr _g_simple_wireframe;
    };
}
