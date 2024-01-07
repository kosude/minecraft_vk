/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "resource_entity.hpp"

#define MINI_CASE_SENSITIVE
#include <mini/ini.h>

#include <filesystem>

namespace mcvk::ResourceMgr {
    class ResourceManager {
    public:
        ResourceManager(const std::filesystem::path &basedir);
        ~ResourceManager();

        template<typename RT>
        bool Load(const std::string &name, RT &res) const;

        inline std::string GetShaderResourcesDir() const { return _base.string() + "/shaders/"; }

    private:
        bool _ReadConfigFile(const std::filesystem::path &path, mINI::INIStructure &ini) const;

        std::filesystem::path _base;
    };

    template<>
    bool ResourceManager::Load<ShaderResource>(const std::string &name, ShaderResource &res) const;
}
