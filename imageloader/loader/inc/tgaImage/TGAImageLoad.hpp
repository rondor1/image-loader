#pragma once

#include <memory>
#include <string_view>
#include <variant>

#include "TGAImage.hpp"

namespace imageloader
{
    class TGAImageLoaderImpl;

    enum class compressionStatus
    {
        NO,
        YES
    };

    class TGAImageLoader
    {
        public:
            TGAImageLoader();
            ~TGAImageLoader();

            std::variant<TGAImage*, ErrorCodes> loadImage(const std::string_view& imagePath);
            std::variant<std::string, ErrorCodes> storeImage(std::string_view& imagePath, const TGAImage& image);
            std::variant<std::string, ErrorCodes> storeImage(std::string_view& imagePath, const TGAImage& image, const compressionStatus& status);

        private:
            bool verifyDirectoryExistence(std::string& imagePath);

        private:
            std::unique_ptr<TGAImageLoaderImpl> d_ptr;
    };

} // namespace imageloader
