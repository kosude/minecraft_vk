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

        inline const VkDevice &GetDevice() const { return _device; }
        inline const VkPhysicalDeviceProperties &GetProperties() const { return _properties; }
        inline const VkCommandPool &GetGraphicsCommandPool() const { return _graphics_command_pool; }
        inline const VkCommandPool &GetTransferCommandPool() const { return _transfer_command_pool; }
        inline const VkQueue &GetGraphicsQueue() const { return _graphics_queue; }
        inline const VkQueue &GetPresentQueue() const { return _present_queue; }
        inline const VkQueue &GetTransferQueue() const { return _transfer_queue; }
        inline SwapChainSupportDetails SwapchainSupportDetails() const { return _QuerySwapChainSupport(_physical_device); }
        inline QueueFamilyIndices FindQueueFamilyIndices() const { return _FindQueueFamilies(_physical_device); }

        uint32_t FindMemoryType(uint32_t filter, VkMemoryPropertyFlags properties) const;
        VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

    private:
        void _PickPhysicalDevice();
        void _CreateLogicalDevice();
        void _CreateCommandPools();

        bool _CheckDeviceSuitable(VkPhysicalDevice device);
        QueueFamilyIndices _FindQueueFamilies(VkPhysicalDevice device) const;
        bool _CheckExtensionSupport(VkPhysicalDevice device) const;
        SwapChainSupportDetails _QuerySwapChainSupport(VkPhysicalDevice device) const;
        VkPhysicalDeviceFeatures _GetRequiredDeviceFeatures() const;

        const Window &_window;
        const VkInstance &_instance;
        const VkSurfaceKHR &_surface;

        VkPhysicalDevice _physical_device;
        VkPhysicalDeviceProperties _properties;

        VkDevice _device;
        VkQueue _graphics_queue;
        VkQueue _present_queue;
        VkQueue _transfer_queue;

        QueueFamilyIndices _queue_families;

        VkCommandPool _graphics_command_pool;
        VkCommandPool _transfer_command_pool;

        const std::vector<const char *> _extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    };
}
