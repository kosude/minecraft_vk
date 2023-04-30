/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __utils__file_io_hpp
#define __utils__file_io_hpp

#include <iostream>
#include <vector>

namespace VKGame::Utils {
    /**
     * @brief Read binary file from filename
     */
    std::vector<char> ReadBinaryFile(
        const std::string &filename
    );
}

#endif
