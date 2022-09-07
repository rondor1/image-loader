#include "tgaImage/TGAImageLoad.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>

#include <iostream>

namespace imageloader
{

    enum TYPE_FORMAT : std::uint8_t
    {
        UNCOMPRESSED_RGB = 2,
        UNCOMPRESSED_BW = 3,
        COMPRESSED_RGB = 10,
        COMPRESSED_BW = 11
    };

    class TGAImageLoaderImpl
    {
        public:

        std::variant<TGAImage*, ErrorCodes> loadImage(const std::string_view& imagePath)
        {
            std::ifstream inputFile(imagePath.data(), std::ios::binary);

            if(!inputFile.is_open())
            {
                return ErrorCodes::UnableToOpenImage;
            }

            TGAHeader header{};
            inputFile.read(reinterpret_cast<char*>(&header), sizeof(header));

            if(!inputFile.good())
            {
                inputFile.close();
                return ErrorCodes::InvalidReadOperation;
            }

            const auto width = header.width;
            const auto height = header.height;
            const auto bpp = (header.bitsperpixel)>>3;

            const auto imageBufferSize = width*height*bpp;

            auto image = std::vector<std::uint8_t>(imageBufferSize, 0);
            image.reserve(imageBufferSize);

            if(header.imagetypecode == TYPE_FORMAT::UNCOMPRESSED_RGB ||
               header.imagetypecode == TYPE_FORMAT::UNCOMPRESSED_BW)
            {
                inputFile.read(reinterpret_cast<char*>(image.data()), imageBufferSize);
                if(!inputFile.good())
                {
                    inputFile.close();
                    return ErrorCodes::InvalidReadOperation;
                }
            }
            else if(header.imagetypecode == TYPE_FORMAT::COMPRESSED_RGB ||
                    header.imagetypecode == TYPE_FORMAT::COMPRESSED_BW)
            {
                ///TODO: Handle uncompressing the data

            }

            return new TGAImage{width, height, bpp, header, image};
        }

        std::variant<std::string, ErrorCodes> storeImage(const std::string_view& imagePath, const TGAImage& image)
        {
            std::ofstream outputFile(imagePath.data(), std::ios::binary | std::ios::out);
            if(!outputFile.is_open())
            {
                return ErrorCodes::UnableToOpenImage;
            }

            auto header = image.getHeader();
            outputFile.write(reinterpret_cast<char*>(&header), sizeof(header));

            if(!outputFile.good())
            {
                outputFile.close();
                if(std::filesystem::exists(imagePath.data()))
                {
                    std::filesystem::remove(imagePath.data());
                }

                return ErrorCodes::InvalidWriteOperation;
            }

            outputFile.write(reinterpret_cast<char*>(image.data()), image.dataSize());
            if(!outputFile.good())
            {
                outputFile.close();
                if(std::filesystem::exists(imagePath.data()))
                {
                    std::filesystem::remove(imagePath.data());
                }

                return ErrorCodes::InvalidWriteOperation;
            }

            outputFile.close();
            return std::string{imagePath};
        }

        private:

        std::vector<std::uint8_t> decompressRunLength(std::ifstream& inputFile, const TGAImage& image)
        {
            ///TODO: Handle RLE

            return {};
        }
    };

    TGAImageLoader::TGAImageLoader() : d_ptr{new TGAImageLoaderImpl}
    {

    }

    std::variant<TGAImage*, ErrorCodes> TGAImageLoader::loadImage(const std::string_view& imagePath)
    {
        if(!std::filesystem::exists(imagePath))
        {
            return ErrorCodes::InvalidaPath;
        }

        return d_ptr->loadImage(imagePath);
    }

    std::variant<std::string, ErrorCodes> TGAImageLoader::storeImage(const std::string_view& imagePath, const TGAImage& image)
    {
        if(!verifyDirectoryExistence(imagePath))
        {
            return ErrorCodes::InvalidaPath;
        }

        return d_ptr->storeImage(imagePath, image);
    }

    std::variant<std::string, ErrorCodes> TGAImageLoader::storeImage(const std::string_view& imagePath, const TGAImage& image,
                                                                      const compressionStatus& status)
    {
        if(compressionStatus::NO == status)
        {
            return storeImage(imagePath, image);
        }

        ///TODO: Handle missing RLE

        return {};
    }


    bool TGAImageLoader::verifyDirectoryExistence(const std::string_view& imagePath)
    {
        //Start of the string + position where '/' is located
        const auto directoryPath = imagePath.substr(0, imagePath.find_last_of('/') + 1);

        return std::filesystem::exists( directoryPath) ? true : std::filesystem::create_directories(directoryPath);
    }

    TGAImageLoader::~TGAImageLoader()
    {
        d_ptr.release();
        d_ptr.reset(nullptr);
    }
} // namespace imageloader
