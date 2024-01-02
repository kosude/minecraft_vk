/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "pipeline.hpp"

#include "utils/log.hpp"

namespace mcvk::Renderer {
    template<typename CreateInfoT>
    Pipeline<CreateInfoT>::Pipeline(const Device &device, const std::vector<ShaderInfo> &shaders)
        : _device{device}, _shader_set{device.GetDevice(), shaders} {
    }

    template<typename CreateInfoT>
    Pipeline<CreateInfoT>::~Pipeline() {
        vkDestroyPipelineLayout(_device.GetDevice(), _layout, nullptr);
        vkDestroyPipeline(_device.GetDevice(), _pipeline, nullptr);
    }

    template class Pipeline<VkGraphicsPipelineCreateInfo>;
}
