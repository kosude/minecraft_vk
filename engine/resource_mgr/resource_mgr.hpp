/*
 * Copyright (c) 2024 Jack Bennett.
 * All Rights Reserved.
 *
 * See the LICENCE file for more information.
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

        static std::vector<std::string> GetAllFilenamesInDir(const std::string &dir);

        template<typename RT>
        bool Load(const std::string &name, RT &res) const;

        inline std::string GetMaterialResourcesDir() const { return _base.string() + "/materials/"; }
        inline std::string GetModelResourcesDir() const { return _base.string() + "/models/"; }
        inline std::string GetPipelineResourcesDir() const { return _base.string() + "/pipelines/"; }
        inline std::string GetShaderResourcesDir() const { return _base.string() + "/shaders/"; }

    private:
        bool _ReadConfigFile(const std::filesystem::path &path, mINI::INIStructure &ini) const;

        std::filesystem::path _base;
    };

    template<>
    bool ResourceManager::Load<MaterialResource>(const std::string &name, MaterialResource &res) const;
    template<>
    bool ResourceManager::Load<ModelResource>(const std::string &name, ModelResource &res) const;
    template<>
    bool ResourceManager::Load<PipelineResource>(const std::string &name, PipelineResource &res) const;
    template<>
    bool ResourceManager::Load<ShaderResource>(const std::string &name, ShaderResource &res) const;
}
