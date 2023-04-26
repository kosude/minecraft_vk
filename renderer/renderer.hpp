/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __renderer__renderer_hpp
#define __renderer__renderer_hpp

#include <volk/volk.h>

#include <vector>
#include <iostream>

namespace VKGame::Renderer {
    class Window;

    /**
     * @brief Game renderer (Vulkan)
     */
    class Renderer {
    private:
        VkInstance _instance;

        VkPhysicalDevice _physical_device;
        VkDevice _device;
        VkQueue _graphics_queue;
        VkQueue _present_queue;

#       ifdef DEBUG
            VkDebugUtilsMessengerEXT _debug_messenger;
#       endif

        VkSurfaceKHR _main_surface;

        VkSwapchainKHR _main_swapchain;
        std::vector<VkImage> _main_swapchain_images;
        VkFormat _main_swapchain_image_format;
        VkExtent2D _main_swapchain_extent;

        void _CreateInstance( // also creates a debug messenger if in debug config
            VkInstance *instance
        );
        void _CreateSurface(
            const Window &window,
            VkSurfaceKHR *surface
        );
        void _CreateLogicalDevice( // also retrieves queue handles
            VkDevice *device
        );

    public:
        /**
         * @brief Construct a new game renderer
         *
         * @param main_window Main game window
         */
        Renderer(
            const Window &main_window
        );

        /**
         * @brief Destroy the game renderer -- clean up Vulkan objects, etc.
         */
        void Destroy();
    };
}

#endif
