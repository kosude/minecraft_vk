/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/window.hpp"

#include <volk/volk.h>

#include <vector>

namespace mcvk::Renderer {
    class InstanceManager {
    public:
        InstanceManager(const Window &window);
        ~InstanceManager();

        InstanceManager(const InstanceManager &) = delete;
        InstanceManager &operator=(const InstanceManager &) = delete;

        const VkInstance &GetInstance() const { return _instance; }
        const VkSurfaceKHR &GetSurface() const { return _surface; }

    private:
        void _CreateInstance();
        void _CreateDebugMessenger();
        void _CreateSurface(const Window &window);

        std::vector<const char *> _GetRequiredExtensions();
        bool _CheckExtensionsSupport(const std::vector<const char *> &required);
        bool _CheckValidationLayerSupport();
        void _PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &create_info);

        VkInstance _instance;
        VkSurfaceKHR _surface;

#       ifdef DEBUG
            VkDebugUtilsMessengerEXT _debug_messenger;
            const std::vector<const char *> _validation_layers = { "VK_LAYER_KHRONOS_validation" };
#       endif
    };
}
