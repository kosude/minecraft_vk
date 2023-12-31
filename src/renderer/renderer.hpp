/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/device.hpp"
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
        void _CreateInstance();
        void _CreateDebugMessenger();
        void _CreateSurface();
        void _CreateDevice();

        std::vector<const char *> _GetRequiredExtensions();
        bool _CheckExtensionsSupport(const std::vector<const char *> &required);
        bool _CheckValidationLayerSupport();
        void _PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &create_info);

        const Window &_window;

        VkInstance _instance;

        VkSurfaceKHR _surface;
        Device _device;

#       ifdef DEBUG
            VkDebugUtilsMessengerEXT _debug_messenger;
            const std::vector<const char *> _validation_layers = { "VK_LAYER_KHRONOS_validation" };
#       endif
    };
}
