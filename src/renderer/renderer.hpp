/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include <volk/volk.h>

#include <vector>

namespace mcvk::Renderer {
    class Renderer {
    public:
        Renderer();
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

    private:
        void _CreateInstance();

        std::vector<const char *> _GetRequiredExtensions();
        bool _CheckExtensionsSupport(const std::vector<const char *> &required);
        bool _CheckValidationLayerSupport();
        void _PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &create_info);

        VkInstance _instance;

#       ifdef DEBUG
            VkDebugUtilsMessengerEXT _debug_messenger;
            const std::vector<const char *> _validation_layers = { "VK_LAYER_KHRONOS_validation" };
#       endif
    };
}
