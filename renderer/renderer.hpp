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
#include <memory>

#include "renderer/buffer/vertex_buffer.hpp"
#include "renderer/buffer/index_buffer.hpp"
#include "renderer/graphics_pipeline.hpp"

namespace MCVK::Renderer {
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
        VkQueue _transfer_queue;

#       ifdef DEBUG
            VkDebugUtilsMessengerEXT _debug_messenger;
#       endif

        VkSurfaceKHR _main_surface;

        VkSwapchainKHR _main_swapchain;
        std::vector<VkImage> _main_swapchain_images;
        std::vector<VkImageView> _main_swapchain_image_views;
        VkFormat _main_swapchain_image_format;
        VkExtent2D _main_swapchain_extent;
        std::vector<VkFramebuffer> _main_swapchain_framebuffers;

        // using a smart ptr to initialise this member later on in the constructor instead of straight away
        std::unique_ptr<GraphicsPipeline> _graphics_pipeline;

        // TODO: stop hardcoding in this file
        std::unique_ptr<Buffer::VertexBuffer> _vertex_buffer;
        std::unique_ptr<Buffer::IndexBuffer> _index_buffer;

        VkCommandPool _draw_command_pool;
        std::vector<VkCommandBuffer> _draw_command_buffers;
        VkCommandPool _transfer_command_pool;

        std::vector<VkSemaphore> _image_available_semaphores;
        std::vector<VkSemaphore> _render_complete_sempahores;
        std::vector<VkFence> _in_flight_fences;

        uint32_t _current_frame = 0;

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
        void _CreateFramebuffers(
            const VkDevice &device
        );
        void _CreateCommandPools(
            const VkDevice &device,
            const VkPhysicalDevice &physical_device,
            const VkSurfaceKHR &surface
        );
        void _CreateCommandBuffers(
            const VkDevice &device
        );
        void _RecordDrawCommandBuffer( // framebuffer_index = the framebuffer that we want to write to (equal to swapchain image views)
            const uint32_t &framebuffer_index
        );
        void _CreateSynchronisationObjects(
            const VkDevice &device
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

        /**
         * @brief Draw function
         */
        void Draw();

        /**
         * @brief Wait for renderer device(s) to be idle.
         */
        void DeviceWaitIdle();
    };
}

#endif
