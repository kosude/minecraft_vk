/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/device.hpp"
#include "renderer/shader_set.hpp"

namespace mcvk::Renderer {
    class GraphicsPipeline {
    public:
        GraphicsPipeline(const Device &device, const std::vector<ShaderInfo> &shaders);
        ~GraphicsPipeline();

        GraphicsPipeline(const GraphicsPipeline &) = delete;
        GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;

    private:
        const Device &_device;

        ShaderSet _shader_set;
    };
}
