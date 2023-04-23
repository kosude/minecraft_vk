/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#include <algorithm>

#include "renderer/renderer.hpp"
#include "renderer/window.hpp"
#include "utils/log.hpp"

#include "vk_extension.hpp"

namespace VKGame::Renderer {
    std::vector<const char *> VkExtensionManager::GetRequiredLayers() {
        std::vector<const char *> required_layers = {
#           ifdef DEBUG
                "VK_LAYER_KHRONOS_validation"
#           endif
        };

        return required_layers;
    }

    std::vector<const char *> VkExtensionManager::GetRequiredInstanceExtensions() {
        std::vector<const char *> required_exts = {
            VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
#           ifdef DEBUG
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#           endif
        };

        uint32_t glfw_ext_count = 0;
        const char *const *glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

        for (uint32_t i = 0; i < glfw_ext_count; i++) {
            required_exts.push_back(glfw_exts[i]);
        }

        return required_exts;
    }

    void VkExtensionManager::CheckLayerNames(const std::vector<const char *> &layers, const VkExtensionImportance &importance) {
        uint32_t supported_layer_count = 0;
        vkEnumerateInstanceLayerProperties(&supported_layer_count, nullptr);
        std::vector<VkLayerProperties> supported_layers(supported_layer_count);
        vkEnumerateInstanceLayerProperties(&supported_layer_count, supported_layers.data());

        // create vector with layer names instead of layer properties
        std::vector<std::string> supported_layer_names;
        for (VkLayerProperties sl : supported_layers) {
            supported_layer_names.emplace_back(sl.layerName);
        }

        for (std::string l : layers) {
            if (std::find(supported_layer_names.begin(), supported_layer_names.end(), l) == supported_layer_names.end()) {
                switch (importance) {
                    case VkExtensionImportance::Required:
                        VKGame::Utils::Error(std::string{"Required Vulkan layer "} + l + " not found or supported");
                        break;
                    case VkExtensionImportance::Optional:
                        VKGame::Utils::Warn(std::string{"Optional Vulkan layer "} + l + " not found or supported");
                        break;
                }
            }
        }
    }

    void VkExtensionManager::CheckInstanceExtensionNames(const std::vector<const char *> &exts, const VkExtensionImportance &importance) {
        uint32_t supported_ext_count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &supported_ext_count, nullptr);
        std::vector<VkExtensionProperties> supported_exts(supported_ext_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &supported_ext_count, supported_exts.data());

        // create vector with ext names instead of layer properties
        std::vector<std::string> supported_ext_names;
        for (VkExtensionProperties se : supported_exts) {
            supported_ext_names.emplace_back(se.extensionName);
        }

        for (std::string e : exts) {
            if (std::find(supported_ext_names.begin(), supported_ext_names.end(), e) == supported_ext_names.end()) {
                switch (importance) {
                    case VkExtensionImportance::Required:
                        VKGame::Utils::Error(std::string{"Required Vulkan instance extension "} + e + " not found or supported");
                        break;
                    case VkExtensionImportance::Optional:
                        VKGame::Utils::Warn(std::string{"Optional Vulkan instance extension "} + e + " not found or supported");
                        break;
                }

            }
        }
    }
}
