/*
 *   Copyright (c) 2023 Jack Bennett
 *   All rights reserved.
 *
 *   Please see the LICENCE file for more information.
 */

#include <cutils/cio/cio.h>
#include <cutils/cio/cioenum.h>

#include <iostream>
#include <ctime>

#include "log.hpp"

/**
 * @brief Get the current date and (if specified) time
 *
 * @param with_time True to append time
 * @return String containing date <and time> in YYYY-MM-DD <hh:mm:ss> format
 */
static std::string __GetCurrentDateTime(const bool &with_time) {
    std::time_t t = std::time(nullptr);
    std::tm *tm = std::localtime(&t);

    char buf[64];

    if (tm) {
        if (with_time) {
            std::strftime(buf, 64, "%Y-%m-%d %X", tm);
        } else {
            std::strftime(buf, 64, "%Y-%m-%d", tm);
        }

        return std::string { buf };
    }

    throw std::runtime_error("Date/time get failed");
}

namespace VKGame::Utils {
    void Log(const std::string &msg) {
        std::cout << "[" << __GetCurrentDateTime(true) << "] " << msg << std::endl;
    }

    void Note(const std::string &msg) {
        ciocolstateset(CIOCOL_BLUE, 0xff, stdout);
        std::cout << "[" << __GetCurrentDateTime(true) << "] " << msg << std::endl;
        ciocolstatedef(stdout);
    }

    void Warn(const std::string &msg) {
        ciocolstateset(CIOCOL_YELLOW, 0xff, stdout);
        std::cout << "[" << __GetCurrentDateTime(true) << "] WARN: " << msg << std::endl;
        ciocolstatedef(stdout);
    }

    void Error(const std::string &msg) {
        ciocolstateset(CIOCOL_RED, 0xff, stdout);
        std::cout << "[" << __GetCurrentDateTime(true) << "] ERROR: " << msg << std::endl;
        ciocolstatedef(stdout);
    }
}
