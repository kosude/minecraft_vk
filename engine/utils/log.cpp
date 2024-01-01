/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "log.hpp"

#include <cio.h>
#include <cioenum.h>

#include <ctime>

static std::string __GetCurrentDateTime(const bool with_time);

namespace mcvk::Utils {
    void ResetLogColour() {
        ciocolstatedef(stdout);
    }

    void Log(const std::string &msg) {
        // verbose messages not shown in release
#       ifdef DEBUG
            std::cout << "[" << __GetCurrentDateTime(true) << "] DEBUG: " << msg << std::endl;
#       endif
    }

    void Info(const std::string &msg) {
        std::cout << "[" << __GetCurrentDateTime(true) << "] " << msg << std::endl;
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

    void Fatal(const std::string &msg, bool except) {
        ciocolstateset(CIOCOL_RED, 0xff, stdout);
        std::cout << "[" << __GetCurrentDateTime(true) << "] FATAL: " << msg << std::endl;
        ciocolstatedef(stdout);

        if (except) {
            throw new std::runtime_error(msg);
        }
    }
}

static std::string __GetCurrentDateTime(const bool with_time) {
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
