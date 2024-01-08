/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/shader_set.hpp"

#include <tiny_obj_loader.h>
#include <volk/volk.h>

#include <vector>

namespace mcvk::ResourceMgr {
    struct GenericResource {
        std::string name;
    };

    struct ModelResource : public GenericResource {
        tinyobj::attrib_t to_attrib;
        std::vector<tinyobj::shape_t> to_shapes;
        std::vector<tinyobj::material_t> to_materials;
    };

    struct PipelineResource : public GenericResource {
        enum class Type {
            Graphics,
            Compute,
            RayTracing,
        } type;

        std::string shader_name;

        VkPolygonMode polygon_mode{VK_POLYGON_MODE_FILL};
        VkCullModeFlags cull_mode{VK_CULL_MODE_NONE};
    };

    struct ShaderResource : public GenericResource {
        std::vector<Renderer::ShaderInfo> shaders;
    };
}
