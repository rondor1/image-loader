#pragma once

#include <cstddef>

namespace imageloader
{
    enum class [[nodiscard]] ErrorCodes
    {
        IndexOutOfRange,
        InvalidaPath,
        UnableToOpenImage,
        InvalidReadOperation,
        InvalidWriteOperation
    };
} // namespace imageloader
