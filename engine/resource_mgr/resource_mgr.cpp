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

    std::vector<std::string> ResourceManager::GetAllFilenamesInDir(const std::string &dir) {
        std::vector<std::string> confs;
        for (const auto &entry : std::filesystem::directory_iterator(dir)) {
            confs.push_back(entry.path().filename());
        }
        return confs;
    }

    template<>
    bool ResourceManager::Load<MaterialResource>(const std::string &name, MaterialResource &res) const {
        mINI::INIStructure ini;
        if (!_ReadConfigFile(GetMaterialResourcesDir() / std::filesystem::path{name}, ini)) {
            return false;
        }


        // detail

        if (!ini.has("detail")) {
            Utils::Error("Invalid material: [detail] section is required.");
            return false;
        }
        auto detail_sect = ini.get("detail");

        res.name = detail_sect.get("name");


        // colour map

        if (!ini.has("textures")) {
            Utils::Error("Invalid material: [textures] section is required.");
            return false;
        }
        auto textures_sect = ini.get("textures");

        std::string colourmap_path = GetMaterialResourcesDir() / std::filesystem::path{textures_sect.get("colour")};
        res.colourmap = LoadImage(colourmap_path);
        if (!res.colourmap->bytes) {
            Utils::Error("Failed to load material: failed to load colour map image");
        }


        Utils::Info("Loaded material \"" + res.name + "\"");
        return true;
    }

    template<>
    bool ResourceManager::Load<ModelResource>(const std::string &name, ModelResource &res) const {
        mINI::INIStructure ini;
        if (!_ReadConfigFile(GetModelResourcesDir() / std::filesystem::path{name}, ini)) {
            return false;
        }


        // detail

        if (!ini.has("detail")) {
            Utils::Error("Invalid model: [detail] section is required.");
            return false;
        }
        auto detail_sect = ini.get("detail");

        res.name = detail_sect.get("name");


        // model

        if (!ini.has("model")) {
            Utils::Error("Invalid model: [model] section is required.");
            return false;
        }
        auto model_sect = ini.get("model");

        if (model_sect.has("obj")) {
            std::filesystem::path path = GetModelResourcesDir() / std::filesystem::path{model_sect.get("obj")};

            std::string warn, err;
            if (!tinyobj::LoadObj(&res.to_attrib, &res.to_shapes, &res.to_materials, &warn, &err, path.c_str())) {
                Utils::Error("Failed to load model resource: " + warn + err);
                return false;
            }
        }


        Utils::Info("Loaded model \"" + res.name + "\"");
        return true;
    }

    template<>
    bool ResourceManager::Load<PipelineResource>(const std::string &name, PipelineResource &res) const {
        mINI::INIStructure ini;
        if (!_ReadConfigFile(GetPipelineResourcesDir() / std::filesystem::path{name}, ini)) {
            return false;
        }


        // detail

        if (!ini.has("detail")) {
            Utils::Error("Invalid pipeline: [detail] section is required.");
            return false;
        }
        auto detail_sect = ini.get("detail");

        res.name = detail_sect.get("name");

        std::string type_str = detail_sect.get("type");
        if (type_str == "graphics") {
            res.type = PipelineResource::Type::Graphics;
        } else if (type_str == "compute") {
            res.type = PipelineResource::Type::Compute;
        } else if (type_str == "ray_tracing") {
            res.type = PipelineResource::Type::RayTracing;
        } else {
            Utils::Error("Invalid pipeline: \"" + type_str + "\" is not a valid type.");
        }


        // shaders

        if (!ini.has("shaders")) {
            Utils::Error("Invalid pipeline: [shaders] section is required.");
            return false;
        }
        auto shaders_sect = ini.get("shaders");

        res.shader_name = shaders_sect.get("shader");


        // rasterization

        if (ini.has("rasterization")) {
            auto raster_sect = ini.get("rasterization");

            std::string cull_mode_str = raster_sect.get("cull_mode");
            if (cull_mode_str == "none") {
                res.cull_mode = VK_CULL_MODE_NONE;
            } else if (cull_mode_str == "back") {
                res.cull_mode = VK_CULL_MODE_BACK_BIT;
            } else if (cull_mode_str == "front") {
                res.cull_mode = VK_CULL_MODE_FRONT_BIT;
            } else if (cull_mode_str == "front_and_back") {
                res.cull_mode = VK_CULL_MODE_FRONT_AND_BACK;
            } else {
                Utils::Error("Invalid pipeline: \"" + cull_mode_str + "\" is not a valid fragment culling mode.");
            }

            std::string polygon_mode_str = raster_sect.get("polygon_mode");
            if (polygon_mode_str == "fill") {
                res.polygon_mode = VK_POLYGON_MODE_FILL;
            } else if (polygon_mode_str == "line") {
                res.polygon_mode = VK_POLYGON_MODE_LINE;
            } else if (polygon_mode_str == "point") {
                res.polygon_mode = VK_POLYGON_MODE_POINT;
            } else {
                Utils::Error("Invalid pipeline: \"" + polygon_mode_str + "\" is not a valid polygon fill mode.");
            }
        }


        Utils::Info("Loaded pipeline \"" + res.name + "\"");
        return true;
    }

    template<>
    bool ResourceManager::Load<ShaderResource>(const std::string &name, ShaderResource &res) const {
        mINI::INIStructure ini;
        if (!_ReadConfigFile(GetShaderResourcesDir() / std::filesystem::path{name}, ini)) {
            return false;
        }


        // detail

        if (!ini.has("detail")) {
            Utils::Error("Invalid shader: [detail] section is required.");
            return false;
        }
        auto detail_sect = ini.get("detail");

        res.name = detail_sect.get("name");


        // spirv

        if (!ini.has("spirv")) {
            Utils::Error("Invalid shader: [spirv] section is required.");
            return false;
        }
        auto spirv_sect = ini.get("spirv");

        res.shaders.clear();
        for (const auto &[stage, spv] : spirv_sect) {
            Renderer::ShaderStage stageenum = Renderer::StringToShaderStage(stage);
            if (stageenum == Renderer::ShaderStage::Null) {
                Utils::Error("Invalid shader: \"" + stage + "\" is not a valid stage. Skipping this stage.");
                continue;
            }
            res.shaders.push_back({stageenum, std::filesystem::path{GetShaderResourcesDir()} / "spv" / spv});
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
