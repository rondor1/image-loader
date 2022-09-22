#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include "Constants.hpp"
#include "ErrorCodes.hpp"

namespace imageloader
{
    #pragma pack(push,1)
    struct TGAHeader
    {
        std::uint8_t idlenght{};
        std::uint8_t colormaptype{};
        std::uint8_t imagetypecode{};
        std::uint16_t colormaporigin{};
        std::uint16_t colormaplength{};
        std::uint8_t colormapsize{};
        std::uint16_t xorigin{};
        std::uint16_t yorigin{};
        std::uint16_t width{};
        std::uint16_t height{};
        std::uint8_t bitsperpixel{};
        std::uint8_t imagedescriptor{};
    };
    #pragma pack(pop)

    struct TGAColor
    {
        std::array<std::uint8_t, imageloader::tgaimage::constants::NUM_OF_CHANNELS> bgra{};
        std::uint8_t bpp{};

        TGAColor() = default;
        TGAColor(const std::uint8_t& r, const std::uint8_t& g, const std::uint8_t& b,const std::uint8_t& a);
        TGAColor(const std::uint8_t* channels, const std::uint8_t& bpp);

        std::variant<std::uint8_t*, ErrorCodes> getIndexedValue(const int& index);

        private:
            std::uint8_t& operator[](const int& index);
    };

    class TGAImageImpl;

    class TGAImage
    {
        public:
            TGAImage();
            TGAImage(const int& width, const int& height, const int& bpp, const TGAHeader& header,const std::vector<std::uint8_t>& imageData);
            ~TGAImage();
            TGAImage(const TGAImage& rhs);
            TGAImage(TGAImage&& rhs);

            TGAImage& operator=(const TGAImage& image);

            int width() const;
            int height() const;
            int bitsPerPixel() const;
            int dataSize() const;
            std::uint8_t* data() const;


            std::variant<TGAColor, ErrorCodes> color(const int& x, const int& y) const;
            std::optional<ErrorCodes> setColor(const int& x, const int& y, const TGAColor& colorValue);
            void setHeader(const TGAHeader&& header);
            TGAHeader getHeader() const;

        private:
            std::unique_ptr<TGAImageImpl> d_ptr;
    };

} // namespace imageloader
