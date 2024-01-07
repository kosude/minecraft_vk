/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/shader_set.hpp"

#include <volk/volk.h>

#include <vector>

namespace mcvk::ResourceMgr {
    struct GenericResource {
        std::string name;
    };

    struct PipelineResource : public GenericResource {
        enum class Type {
            Graphics,
            Compute,
            RayTracing,
        } type;

        std::string shader_name;

        VkCullModeFlags cull_mode{VK_CULL_MODE_NONE};
    };

    struct ShaderResource : public GenericResource {
        std::vector<Renderer::ShaderInfo> shaders;
    };
}
