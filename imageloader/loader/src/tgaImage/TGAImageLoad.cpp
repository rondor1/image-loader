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
                auto tempData = decompressRunLength(inputFile, header);
                image = std::get<std::vector<uint8_t>>(tempData);
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

        std::variant<std::vector<std::uint8_t>, ErrorCodes> decompressRunLength(std::ifstream& inputFile, const TGAHeader& header)
        {
            const auto pixelCount = header.width*header.height;
            auto currentPixel = 0;
            auto currentByte = 0;
            const auto bytesPerPixel = header.bitsperpixel>>3;
            TGAColor color;

            std::vector<std::uint8_t> data(pixelCount*bytesPerPixel, 0);

            while(currentPixel < pixelCount)
            {
                auto chunkHeader = static_cast<std::uint8_t>(inputFile.get());

                if(!inputFile.good())
                {
                    return ErrorCodes::InvalidReadOperation;
                }

                if(!(chunkHeader & 0x80))
                {
                    ++chunkHeader;
                    for(auto iter = 0; iter < chunkHeader; ++iter)
                    {
                        inputFile.read(reinterpret_cast<char*>(&color), (header.bitsperpixel>>3));
                        if(!inputFile.good())
                        {
                            return ErrorCodes::InvalidReadOperation;
                        }

                        for(auto i = 0; i < bytesPerPixel; ++i)
                        {
                            data[currentByte++] = color.bgra[1];
                        }
                        ++currentPixel;

                        if(currentPixel > pixelCount)
                        {
                            return ErrorCodes::InvalidReadOperation;
                        }
                    }
                }
                else
                {
                    chunkHeader -= 127;
                    inputFile.read(reinterpret_cast<char*>(&color.bgra), bytesPerPixel);
                    if(!inputFile.good())
                    {
                        return ErrorCodes::InvalidReadOperation;
                    }

                    for(auto i = 0; i < chunkHeader; ++i)
                    {
                        for(auto j = 0; j < bytesPerPixel; ++j)
                        {
                            data[currentByte++] = color.bgra[j];
                        }
                        ++currentPixel;

                        if(currentPixel > pixelCount)
                        {
                            return ErrorCodes::InvalidReadOperation;
                        }
                    }
                }
            }

            return data;
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
        std::ofstream out = std::ofstream(imagePath.data(), std::ios::out | std::ios::binary);
        auto header = image.getHeader();
        out.write(reinterpret_cast<char*>(&header), sizeof(header));
        const std::uint8_t max_chunk_length = 128;
        size_t npixels = image.getHeader().height*image.getHeader().width;
        size_t curpix = 0;
        auto bpp = image.bitsPerPixel();
        auto data = image.data();
        while (curpix<npixels) {
            size_t chunkstart = curpix*bpp;
            size_t curbyte = curpix*bpp;
            std::uint8_t run_length = 1;
            bool raw = true;
            while (curpix+run_length<npixels && run_length<max_chunk_length) {
                bool succ_eq = true;
                for (int t=0; succ_eq && t<bpp; t++)
                    succ_eq = (data[curbyte+t]==data[curbyte+t+bpp]);
                curbyte += bpp;
                if (1==run_length)
                    raw = !succ_eq;
                if (raw && succ_eq) {
                    run_length--;
                    break;
                }
                if (!raw && !succ_eq)
                    break;
                run_length++;
            }
            curpix += run_length;
            out.put(raw?run_length-1:run_length+127);
            if (!out.good()) {
                std::cerr << "can't dump the tga file\n";
                return ErrorCodes::InvalidWriteOperation;
            }
            out.write(reinterpret_cast<const char *>(data+chunkstart), (raw?run_length*bpp:bpp));
            if (!out.good()) {
                std::cerr << "can't dump the tga file\n";
                return ErrorCodes::InvalidWriteOperation;
            }
        }

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
