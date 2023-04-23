/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#pragma once
#ifndef __utils__log_hpp
#define __utils__log_hpp

#include <iostream>

namespace VKGame::Utils {
    /**
     * @brief Log a message to stdout
     *
     * @param msg Message string
     */
    void Log(
        const std::string &msg
    );

    /**
     * @brief Log a notification message to stdout
     *
     * @param msg Message string
     */
    void Note(
        const std::string &msg
    );

    /**
     * @brief Log a warning message to stdout
     *
     * @param msg Message string
     */
    void Warn(
        const std::string &msg
    );

    /**
     * @brief Send an error message to stderr
     *
     * @param msg Message string
     */
    void Error(
        const std::string &msg
    );
}

#endif
