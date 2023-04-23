/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __renderer__vk_extension_hpp
#define __renderer__vk_extension_hpp

#include <vector>

namespace VKGame::Renderer {
    /**
     * @brief Enum of available extension importances
     */
    enum class VkExtensionImportance {
        /** Throw a runtime error if somethign isn't available/supported */
        Required,
        /** Give a warning if something isnt available/supproted */
        Optional
    };

    /**
     * @brief Class for functions with vulkan extensions, layers, features, etc for a Vulkan instance and device(s)
     *
     */
    class VkExtensionManager {
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
         * @brief Check given layer names against available ones
         */
        static void CheckLayerNames(
            const std::vector<const char *> &layers,
            const VkExtensionImportance &importance
        );

        /**
         * @brief Check given layer names against available ones
         */
        static void CheckInstanceExtensionNames(
            const std::vector<const char *> &exts,
            const VkExtensionImportance &importance
        );
    };
}

#endif
