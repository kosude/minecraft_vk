/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#include "file_io.hpp"

#include "utils/log.hpp"

#include <fstream>

namespace MCVK::Utils {
    std::vector<char> ReadBinaryFile(const std::string &filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            Utils::Error("Filesystem io error (reading from path " + filename + ")");
        }

        size_t file_size = (size_t) file.tellg();
        std::vector<char> buf(file_size);

        // read into buf
        file.seekg(0);
        file.read(buf.data(), file_size);

        file.close();
        return buf;
    }
}
