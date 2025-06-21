/*
 * Copyright (c) 2024 Jack Bennett.
 * All Rights Reserved.
 *
 * See the LICENCE file for more information.
 */

#pragma once

#include "renderer/device.hpp"
#include "renderer/shader_set.hpp"

#include <vector>

namespace mcvk::Renderer {
    template<typename CreateInfoT>
    class Pipeline {
    public:
        Pipeline(const Device &device, const std::vector<ShaderInfo> &shaders);
        ~Pipeline();

        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;

        inline const VkPipeline &GetPipeline() const { return _pipeline; }
        inline const VkPipelineLayout &GetPipelineLayout() const { return _layout; }
        inline const CreateInfoT &GetCreateInfo() const { return _info; }

    protected:
        virtual void _BuildLayout() = 0;
        virtual void _BuildCreateInfo() = 0;

        const Device &_device;

        ShaderSet _shader_set;

        VkPipelineLayout _layout;
        VkPipeline _pipeline;

        CreateInfoT _info{};
    };
}
