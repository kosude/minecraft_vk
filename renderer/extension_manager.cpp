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

#include "extension_manager.hpp"

namespace MCVK::Renderer {
    std::vector<const char *> ExtensionManager::GetRequiredLayers() {
        std::vector<const char *> required_layers = {
#           ifdef DEBUG
                "VK_LAYER_KHRONOS_validation"
#           endif
        };

        return required_layers;
    }

    std::vector<const char *> ExtensionManager::GetRequiredInstanceExtensions() {
        std::vector<const char *> required_exts = {
#           ifdef APPLE
                VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
                "VK_KHR_get_physical_device_properties2",
#           endif
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

    std::vector<const char *> ExtensionManager::GetRequiredDeviceExtensions() {
        std::vector<const char*> required_exts = {
#           ifdef APPLE
                "VK_KHR_portability_subset",
#           endif
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        return required_exts;
    }

    VkPhysicalDeviceFeatures ExtensionManager::GetRequiredDeviceFeatures() {
        VkPhysicalDeviceFeatures features = {};

        return features;
    }

    bool ExtensionManager::CheckLayerNames(const std::vector<const char *> &layers, const ExtensionImportance &importance) {
        uint32_t supported_layer_count = 0;
        vkEnumerateInstanceLayerProperties(&supported_layer_count, nullptr);
        std::vector<VkLayerProperties> supported_layers(supported_layer_count);
        vkEnumerateInstanceLayerProperties(&supported_layer_count, supported_layers.data());

        // create vector with layer names instead of layer properties
        std::vector<std::string> supported_layer_names;
        for (const VkLayerProperties &sl : supported_layers) {
            supported_layer_names.emplace_back(sl.layerName);
        }

        for (const std::string &l : layers) {
            if (std::find(supported_layer_names.begin(), supported_layer_names.end(), l) == supported_layer_names.end()) {
                switch (importance) {
                    case ExtensionImportance::Required:
                        MCVK::Utils::Error(std::string{"Required Vulkan layer "} + l + " not found or supported");
                        break;
                    case ExtensionImportance::Optional:
                        MCVK::Utils::Warn(std::string{"Vulkan layer "} + l + " not found or supported");
                        break;
                }

                return false;
            }
        }

        return true;
    }

    bool ExtensionManager::CheckInstanceExtensionNames(const std::vector<const char *> &exts, const ExtensionImportance &importance) {
        uint32_t supported_ext_count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &supported_ext_count, nullptr);
        std::vector<VkExtensionProperties> supported_exts(supported_ext_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &supported_ext_count, supported_exts.data());

        // create vector with ext names instead of ext properties
        std::vector<std::string> supported_ext_names;
        for (const VkExtensionProperties &se : supported_exts) {
            supported_ext_names.emplace_back(se.extensionName);
        }

        for (const std::string &e : exts) {
            if (std::find(supported_ext_names.begin(), supported_ext_names.end(), e) == supported_ext_names.end()) {
                switch (importance) {
                    case ExtensionImportance::Required:
                        MCVK::Utils::Error(std::string{"Required Vulkan instance extension "} + e + " not found or supported");
                        break;
                    case ExtensionImportance::Optional:
                        MCVK::Utils::Warn(std::string{"Vulkan instance extension "} + e + " not found or supported");
                        break;
                }

                return false;
            }
        }

        return true;
    }

    bool ExtensionManager::CheckDeviceExtensionNames(const std::vector<const char *> &exts, const ExtensionImportance &importance,
        const VkPhysicalDevice &device)
    {
        uint32_t supported_ext_count = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &supported_ext_count, nullptr);
        std::vector<VkExtensionProperties> supported_exts(supported_ext_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &supported_ext_count, supported_exts.data());

        // create vector with ext names instead of ext properties
        std::vector<std::string> supported_ext_names;
        for (const VkExtensionProperties &se : supported_exts) {
            supported_ext_names.emplace_back(se.extensionName);
        }

        for (const std::string &e : exts) {
            if (std::find(supported_ext_names.begin(), supported_ext_names.end(), e) == supported_ext_names.end()) {
                switch (importance) {
                    case ExtensionImportance::Required:
                        MCVK::Utils::Error(std::string{"Required Vulkan device extension "} + e + " not found or supported");
                        break;
                    case ExtensionImportance::Optional:
                        MCVK::Utils::Warn(std::string{"Vulkan device extension "} + e + " not found or supported");
                        break;
                }

                return false;
            }
        }

        return true;
    }

    bool ExtensionManager::CheckDeviceFeatures(const VkPhysicalDeviceFeatures &available_features) {
        return true;
    }
}
