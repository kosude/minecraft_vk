/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include <volk/volk.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace mcvk::Renderer {
    enum class ShaderStage {
        Null,
        Vertex,
        Fragment
    };
    VkShaderStageFlagBits ShaderStageToFlagBits(ShaderStage s);
    ShaderStage StringToShaderStage(const std::string &str);

    struct ShaderInfo {
        ShaderStage stage;
        std::string path;
    };

    class ShaderSet {
    public:
        ShaderSet(const VkDevice &device, const std::vector<ShaderInfo> &shaders);
        ~ShaderSet();

        ShaderSet(const ShaderSet &) = delete;
        ShaderSet &operator=(const ShaderSet &) = delete;

        std::vector<VkPipelineShaderStageCreateInfo> BuildShaderStageInfos();

    private:
        std::vector<char> _ReadFile(const std::string &path);
        void _AddShaderModule(ShaderStage stage, const std::vector<char> &src);

        const VkDevice &_device;

        std::unordered_map<VkShaderStageFlagBits, VkShaderModule> _shader_modules;
    };
}
