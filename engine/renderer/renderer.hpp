/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/device.hpp"
#include "renderer/instance_manager.hpp"
#include "renderer/graphics_pipeline.hpp"
#include "renderer/shader_set.hpp"
#include "renderer/window.hpp"

#include <volk/volk.h>

#include <vector>

namespace mcvk::Renderer {
    class Renderer {
    public:
        Renderer(const Window &window);
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

    private:
        std::vector<ShaderInfo> _GetShaders();

        const Window &_window;
        VkSurfaceKHR _surface;

        InstanceManager _instance_mgr;
        Device _device;
        GraphicsPipeline _graphics_pipeline;
    };
}
