#pragma once

#include <cstdint>

namespace utils::constants
{
    enum class LogLevels : std::uint8_t {
        error,
        critical,
        warning,
        debug,
        info
    };

    constexpr auto error = "error";
    constexpr auto critical = "critical";
    constexpr auto warning = "warning";
    constexpr auto debug = "debug";
    constexpr auto info = "info";
} // namespace imageloader::utils
