/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include <bits/stringfwd.h>

namespace mcvk::Utils {
    void ResetLogColour();

    void Log(const std::string &msg);
    void Info(const std::string &msg);
    void Warn(const std::string &msg);
    void Error(const std::string &msg);
    void Fatal(const std::string &msg, bool except = true);
}
