/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/pipeline/graphics_pipeline.hpp"
#include "renderer/device.hpp"

#include <vector>

namespace mcvk::Renderer {
    class PipelineFactory {
    public:
        static void BuildPipelines(const Device &device, const std::vector<GraphicsPipeline *> &pipelines);
    };
}
