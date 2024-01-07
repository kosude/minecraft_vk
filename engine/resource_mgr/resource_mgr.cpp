/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "resource_mgr.hpp"

#include "utils/log.hpp"

namespace mcvk::ResourceMgr {
    ResourceManager::ResourceManager(const std::filesystem::path &basedir)
        : _base{std::filesystem::canonical(basedir)} {
        Utils::Info("Instantiating resource manager for base path: \"" + _base.string() + "\"");
    }

    ResourceManager::~ResourceManager() {
    }

    template<>
    bool ResourceManager::Load<ShaderResource>(const std::string &name, ShaderResource &res) const {
        const std::string path = GetShaderResourcesDir() / std::filesystem::path{name};

        mINI::INIStructure ini;
        if (!_ReadConfigFile(path, ini)) {
            return false;
        }

        if (!ini.has("detail")) {
            Utils::Error("Invalid shader: [detail] section is required.");
            return false;
        }
        auto detail_sect = ini.get("detail");
        if (!ini.has("spirv")) {
            Utils::Error("Invalid shader: [spirv] section is required.");
            return false;
        }
        auto spirv_sect = ini.get("spirv");

        res.name = detail_sect.get("name");

        res.shaders.clear();
        for (const auto &[stage, spv] : spirv_sect) {
            Renderer::ShaderStage stageenum = Renderer::StringToShaderStage(stage);
            if (stageenum == Renderer::ShaderStage::Null) {
                Utils::Error("Invalid shader: \"" + stage + "\" is not a valid stage. Skipping this stage.");
                continue;
            }
            res.shaders.push_back({stageenum, std::filesystem::path(GetShaderResourcesDir()) / "spv" / spv});
        }

        Utils::Info("Loaded shader \"" + res.name + "\" which programs " + std::to_string(res.shaders.size()) + " stages");

        return true;
    }

    bool ResourceManager::_ReadConfigFile(const std::filesystem::path &path, mINI::INIStructure &ini) const {
        mINI::INIFile file{path.string()};

        if (!file.read(ini)) {
            Utils::Error("Failed to read config file at \"" + path.string() + "\"");
            return false;
        }

        return true;
    }
}
