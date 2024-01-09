/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include <string>
#include <memory>

namespace mcvk::ResourceMgr {
    struct ImageLoadResult {
        inline ImageLoadResult(uint8_t *d, int32_t w, int32_t h, int32_t c)
            : bytes(d), width(w), height(h), channels(c) {}
        ~ImageLoadResult();

        uint8_t *bytes;

        int32_t width;
        int32_t height;
        int32_t channels;
    };

    std::unique_ptr<ImageLoadResult> LoadImage(const std::string &path);
}
