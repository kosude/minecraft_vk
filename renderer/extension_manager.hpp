/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __renderer__extension_manager_hpp
#define __renderer__extension_manager_hpp

#include <volk/volk.h>

#include <vector>

namespace VKGame::Renderer {
    /**
     * @brief Enum of available extension importances
     */
    enum class ExtensionImportance {
        /** Throw a runtime error if somethign isn't available/supported */
        Required,
        /** Give a warning if something isnt available/supproted */
        Optional
    };

    /**
     * @brief Class for functions with vulkan extensions, layers, features, etc for a Vulkan instance and device(s)
     *
     */
    class ExtensionManager {
    public:
        /**
         * @brief Get layers required by the game renderer instance
         */
        static std::vector<const char *> GetRequiredLayers();

        /**
         * @brief Get instance extensions required by the game renderer instance
         */
        static std::vector<const char *> GetRequiredInstanceExtensions();

        /**
         * @brief Get device extensions required by the game renderer device(s)
         */
        static std::vector<const char *> GetRequiredDeviceExtensions();

        /**
         * @brief Get device features required by the game renderer device(s)
         */
        static VkPhysicalDeviceFeatures GetRequiredDeviceFeatures();

        /**
         * @brief Check given layer names against available ones
         */
        static bool CheckLayerNames(
            const std::vector<const char *> &layers,
            const ExtensionImportance &importance
        );

        /**
         * @brief Check given instance extension names against available ones
         */
        static bool CheckInstanceExtensionNames(
            const std::vector<const char *> &exts,
            const ExtensionImportance &importance
        );

        /**
         * @brief Check given device extension names against available ones
         */
        static bool CheckDeviceExtensionNames(
            const std::vector<const char *> &exts,
            const ExtensionImportance &importance,
            const VkPhysicalDevice &device
        );

        /**
         * @brief Check given device features against required ones
         */
        static bool CheckDeviceFeatures(
            const VkPhysicalDeviceFeatures &available_features
        );
    };
}

#endif
