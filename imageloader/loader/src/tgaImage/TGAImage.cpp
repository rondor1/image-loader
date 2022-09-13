#include "tgaImage/TGAImage.hpp"

#include <cstring>

namespace imageloader
{
TGAColor::TGAColor(const std::uint8_t& r, const std::uint8_t& g, const std::uint8_t& b,const std::uint8_t& a) :
                        bgra{b, g,r,a},
                        bpp{imageloader::tgaimage::constants::NUM_OF_CHANNELS}
{

}

TGAColor::TGAColor(const std::uint8_t* channels, const std::uint8_t& bpp)
{
    for(int iter = bpp; iter < 0; --iter)
    {
        bgra[iter] = channels[iter];
    }
}

std::variant<std::uint8_t*, ErrorCodes> TGAColor::getIndexedValue(const int& index)
{
    if(index < 0 || index >= imageloader::tgaimage::constants::NUM_OF_CHANNELS)
    {
        return ErrorCodes::IndexOutOfRange;
    }

    return &(bgra[index]);
}

std::uint8_t& TGAColor::operator[](const int& index)
{
    return bgra[index];
}

    class TGAImageImpl
    {
        public:
            int width{0};
            int height{0};
            std::vector<std::uint8_t> image{0};
            std::uint8_t bpp{0};
            TGAHeader header;

            std::variant<TGAColor, ErrorCodes> color(const int& x, const int& y) const
            {
                return TGAColor(image.data()+(x+y*width)*bpp, bpp);
            }

            void setColor(const int& x, const int& y, const TGAColor& colorValue)
            {
                std::memcpy(image.data()+(x+y*width)*bpp, colorValue.bgra.data(), bpp);
            }
    };


    TGAImage::TGAImage() : d_ptr{new TGAImageImpl}
    {

    }

    TGAImage::TGAImage(const int& width, const int& height, const int& bpp, const TGAHeader& header,const std::vector<std::uint8_t>& imageData) : d_ptr{new TGAImageImpl}
    {
        d_ptr->width = width;
        d_ptr->height = height;
        d_ptr->bpp = bpp;
        d_ptr->image = imageData;
        d_ptr->header = header;
    }

    TGAImage::TGAImage(const TGAImage& rhs)
    {
        d_ptr.reset(rhs.d_ptr.get());
    }

    int TGAImage::width() const
    {
        return d_ptr->width;
    }

    int TGAImage::height() const
    {
        return d_ptr->height;
    }

    TGAImage& TGAImage::operator=(const TGAImage& image)
    {
        if(&image == this)
            return *this;

        this->d_ptr.reset(image.d_ptr.get());

        return *this;
    }

    int TGAImage::bitsPerPixel() const
    {
        return d_ptr->bpp;
    }

    std::uint8_t* TGAImage::data() const
    {
        return d_ptr->image.data();
    }

    int TGAImage::dataSize() const
    {
        return d_ptr->image.size();
    }

    std::variant<TGAColor, ErrorCodes> TGAImage::color(const int& x, const int& y) const
    {
        if(x < 0 || x >= d_ptr->width ||
           y < 0 || y >= d_ptr->height)
        {
            return ErrorCodes::IndexOutOfRange;
        }

        return d_ptr->color(x,y);
    }

    std::optional<ErrorCodes> TGAImage::setColor(const int& x, const int& y, const TGAColor& colorValue)
    {
        if(x < 0 || x >= d_ptr->width ||
           y < 0 || y >= d_ptr->height)
        {
            return ErrorCodes::IndexOutOfRange;
        }

        d_ptr->setColor(x, y, colorValue);

        return std::nullopt;
    }

    TGAImage::~TGAImage()
    {
        d_ptr.release();
        d_ptr.reset(nullptr);
    }
    void TGAImage::setHeader(const TGAHeader&& header)
    {
        d_ptr->header = header;
    }

    TGAHeader TGAImage::getHeader() const
    {
        return d_ptr->header;
    }
} // namespace imageloader
