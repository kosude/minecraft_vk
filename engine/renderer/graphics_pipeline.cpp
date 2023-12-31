/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "graphics_pipeline.hpp"

#include "utils/log.hpp"

namespace mcvk::Renderer {
    GraphicsPipeline::GraphicsPipeline(const Device &device, const std::vector<ShaderInfo> &shaders)
        : _device(device), _shader_set(device.LogicalDevice(), shaders) {
        Utils::Info("Hooking up new graphics pipeline with " + std::to_string(shaders.size()) + " shaders...");
    }

    GraphicsPipeline::~GraphicsPipeline() {
    }
}
