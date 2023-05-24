/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __renderer__device_manager_hpp
#define __renderer__device_manager_hpp

#include <volk/volk.h>

#include <vector>
#include <optional>

namespace MCVK::Renderer {
    /**
     * @brief Struct containing queue family indices. Expanded as needed -- -1 means not found.
     */
    struct DeviceQueueFamilyInfo {
    public:
        std::optional<uint32_t> graphics_family_index = -1;
        std::optional<uint32_t> present_family_index = -1;
    };

    /**
     * @brief Class for managing Vulkan devices + physical devices
     */
    class DeviceManager {
    public:
        /**
         * @brief Get physical device queue family indices
         *
         * @param physical_device VkPhysicalDevice object
         * @param surface Surface for device to present to
         */
        static DeviceQueueFamilyInfo GetDeviceQueueFamilyInfo(
            const VkPhysicalDevice &physical_device,
            const VkSurfaceKHR &surface
        );

        /**
         * @brief Check if the given physical device is suitable for game rendering
         *
         * @param physical_device device to check
         * @return true if the device can be used
         */
        static bool IsDeviceSuitable(
            const VkPhysicalDevice &physical_device,
            const VkSurfaceKHR &surface
        );

        /**
         * @brief Assign given device a score based on specs
         *
         * @param physical_device device to score
         */
        static uint64_t ScorePhysicalDevice(
            const VkPhysicalDevice &physical_device
        );

        /**
         * @brief Get optimal physical device out of given options
         *
         * @param choices possible physical devices for use
         */
        static VkPhysicalDevice GetOptimalPhysicalDevice(
            std::vector<VkPhysicalDevice> choices,
            const VkSurfaceKHR &surface
        );
    };
}

#endif
