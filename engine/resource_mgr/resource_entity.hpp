/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/shader_set.hpp"

#include <vector>

namespace mcvk::ResourceMgr {
    struct ShaderResource {
        std::string name;
        std::vector<Renderer::ShaderInfo> shaders;
    };
}
