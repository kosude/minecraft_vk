/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/window.hpp"

#include <optional>
#include <vector>

namespace mcvk::Renderer {
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> surface_formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> present;
        std::optional<uint32_t> compute;
        std::optional<uint32_t> transfer;

        bool isComplete() { return graphics && present && compute && transfer; }
    };

    class Device {
    public:
        Device(const Window &window, const VkInstance &instance, const VkSurfaceKHR &_surface);
        ~Device();

        Device(const Device &) = delete;
        Device &operator=(const Device &) = delete;
        Device(Device &&) = delete;
        Device &operator=(Device &&) = delete;

        void Initialise();
        void Destroy();

        inline const VkPhysicalDeviceProperties &GetProperties() const { return _properties; }

    private:
        void _PickPhysicalDevice();
        void _CreateLogicalDevice();
        void _CreateCommandPools();

        bool _CheckDeviceSuitable(VkPhysicalDevice device);
        QueueFamilyIndices _FindQueueFamilies(VkPhysicalDevice device);
        bool _CheckExtensionSupport(VkPhysicalDevice device);
        SwapChainSupportDetails _QuerySwapChainSupport(VkPhysicalDevice device);
        VkPhysicalDeviceFeatures _GetRequiredDeviceFeatures();

        const Window &_window;
        const VkInstance &_instance;
        const VkSurfaceKHR &_surface;

        VkPhysicalDevice _physical_device;
        VkPhysicalDeviceProperties _properties;

        VkDevice _device;
        VkQueue _graphics_queue;
        VkQueue _present_queue;

        QueueFamilyIndices _queue_families;

        VkCommandPool _graphics_command_pool;
        VkCommandPool _transfer_command_pool;

        const std::vector<const char *> _extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    };
}
