/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "image_load.hpp"

#include "utils/log.hpp"

#include <stb_image.h>

namespace mcvk::ResourceMgr {
    ImageLoadResult::~ImageLoadResult() {
        stbi_image_free(bytes); // equiv to free()
        bytes = nullptr;
    }

    std::unique_ptr<ImageLoadResult> LoadImage(const std::string &path) {
        int32_t width, height, channels;
        stbi_uc *pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (!pixels) {
            const char *err = stbi_failure_reason();
            Utils::Error("Failed to load image from " + path + ": \"" + err + "\"");
        }

        return std::make_unique<ImageLoadResult>(pixels, width, height, channels);
    }
}
