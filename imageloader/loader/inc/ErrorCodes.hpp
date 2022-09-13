#pragma once

#include <cstddef>

namespace imageloader
{
    enum class [[nodiscard]] ErrorCodes
    {
        IndexOutOfRange,
        InvalidPath,
        UnableToOpenImage,
        InvalidReadOperation,
        InvalidWriteOperation
    };
} // namespace imageloader
