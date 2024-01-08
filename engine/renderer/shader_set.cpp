/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "shader_set.hpp"

#include "utils/log.hpp"

#include <fstream>

namespace mcvk::Renderer {
    VkShaderStageFlagBits ShaderStageToFlagBits(ShaderStage s) {
        switch (s) {
            case ShaderStage::Vertex:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderStage::Fragment:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            default:
                return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        }
    }

    ShaderStage StringToShaderStage(const std::string &str) {
        static const std::unordered_map<std::string, ShaderStage> strenum_map = {
            { "vertex", ShaderStage::Vertex },
            { "fragment", ShaderStage::Fragment },
        };
        auto it = strenum_map.find(str);
        if (it != strenum_map.end()) {
            return it->second;
        } else {
            return ShaderStage::Null;
        }
    }

    ShaderSet::ShaderSet(const VkDevice &device, const std::vector<ShaderInfo> &shaders)
        : _device{device}, _shader_modules{} {
        // create modules for each specified shader
        for (ShaderInfo shad : shaders) {
            auto src = _ReadFile(shad.path);
            _AddShaderModule(shad.stage, src);
        }
    }

    ShaderSet::~ShaderSet() {
        for (auto &[_, mod] : _shader_modules) {
            vkDestroyShaderModule(_device, mod, nullptr);
        }
    }

    std::vector<VkPipelineShaderStageCreateInfo> ShaderSet::BuildShaderStageInfos() {
        std::vector<VkPipelineShaderStageCreateInfo> infos(_shader_modules.size());

        uint32_t i = 0;
        for (auto &[stage, mod] : _shader_modules) {
            infos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            infos[i].pNext = nullptr;
            infos[i].flags = 0;
            infos[i].stage = stage;
            infos[i].module = mod;
            infos[i].pName = "main";
            infos[i].pSpecializationInfo = nullptr;

            i++;
        }

        return infos;
    }

    std::vector<char> ShaderSet::_ReadFile(const std::string &path) {
        std::ifstream file{path, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            Utils::Fatal("Failed to open file at path \"" + path + "\"");
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    void ShaderSet::_AddShaderModule(ShaderStage stage, const std::vector<char> &src) {
        VkShaderModuleCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = src.size();
        info.pCode = reinterpret_cast<const uint32_t *>(src.data());

        VkShaderModule mod;
        if (vkCreateShaderModule(_device, &info, nullptr, &mod) != VK_SUCCESS) {
            Utils::Fatal("Failed to create shader module");
        }

        VkShaderStageFlagBits s = ShaderStageToFlagBits(stage);
        _shader_modules[s] = mod;
    }
}
