#include "tgaImage/TGAImageLoad.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>

namespace imageloader
{

    enum TYPE_FORMAT : std::uint8_t
    {
        UNCOMPRESSED_RGB = 2,
        UNCOMPRESSED_BW = 3,
        COMPRESSED_RGB = 10,
        COMPRESSED_BW = 11
    };

    constexpr auto maxChunkLength = 128;
    constexpr auto maxDataLenghtRLE = 127;
    constexpr auto runLengthMask = 0x80;

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
                auto result = decompressRunLength(inputFile, header);
                if(std::holds_alternative<ErrorCodes>(result))
                {
                    return std::get<ErrorCodes>(result);
                }

                image = std::get<std::vector<std::uint8_t>>(result);
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

        std::variant<std::string, ErrorCodes> storeCompressedImage(const std::string_view& imagePath, const TGAImage& image)
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
                return ErrorCodes::InvalidWriteOperation;
            }

            auto result = compressRunLength(outputFile, image.data(), header);
            if(!result.has_value())
            {
                return std::string{imagePath.data()};
            }

            return result.value();
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

                    //Check if chunk is RAW
                    if(!(chunkHeader & runLengthMask))
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
                                data[currentByte++] = color.bgra[i];
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
                        //Handle RLE data
                        chunkHeader -= maxDataLenghtRLE;
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

            std::optional<ErrorCodes> compressRunLength(std::ofstream& outputFile, std::uint8_t* data, const TGAHeader& header)
            {

                if(data == nullptr)
                {
                    return ErrorCodes::InvalidWriteOperation;
                }

                const auto pixelCount = header.width*header.height;
                const auto bytesPerPixel = header.bitsperpixel >> 3;
                auto currentPixel = 0;
                bool isChunkRaw = true;

                while(currentPixel < pixelCount)
                {
                    auto runLengthNumber = 1;
                    auto currentByte = currentPixel*bytesPerPixel;
                    auto chunkStart = currentPixel*bytesPerPixel;

                    while(currentPixel + runLengthNumber < pixelCount && runLengthNumber < maxChunkLength)
                    {
                        bool isSuccessorEqual = true;
                        for(auto iter = 0; isSuccessorEqual && iter < bytesPerPixel; ++iter)
                        {
                            isSuccessorEqual = data[currentByte+iter] == data[currentByte+bytesPerPixel+iter];
                        }
                        currentByte += bytesPerPixel;
                        if(runLengthNumber == 1)
                        {
                            // If chunk is RAw, further checks will terminate the loop, this one just sets the RAW flag to proper state
                            isChunkRaw = !isSuccessorEqual;
                        }

                        //Handle first occurence of a similar chunk, while RAW data is being processed
                        if(isChunkRaw && isSuccessorEqual)
                        {
                            --runLengthNumber;
                            break;
                        }

                        if(!isChunkRaw && !isSuccessorEqual)
                        {
                            break;
                        }

                        ++runLengthNumber;
                    }

                    currentPixel += runLengthNumber;

                    const auto chunkStatusValue = isChunkRaw ? runLengthNumber-1 : runLengthNumber + maxDataLenghtRLE;
                    outputFile.put(chunkStatusValue);
                    if(!outputFile.good())
                    {
                        return ErrorCodes::InvalidWriteOperation;
                    }

                    //In case of a RAW data, write bigger chunk, as size number of raw chunks * bytesPerPixel
                    const auto dataToBeWriten = isChunkRaw ? runLengthNumber*bytesPerPixel : bytesPerPixel;
                    outputFile.write(reinterpret_cast<char*>(data+chunkStart), dataToBeWriten);
                    if(!outputFile.good())
                    {
                        return ErrorCodes::InvalidWriteOperation;
                    }
                }

                return std::nullopt;
            }
    };

    TGAImageLoader::TGAImageLoader() : d_ptr{new TGAImageLoaderImpl}
    {

    }

    std::variant<TGAImage*, ErrorCodes> TGAImageLoader::loadImage(const std::string_view& imagePath)
    {
        if(!std::filesystem::exists(imagePath))
        {
            return ErrorCodes::InvalidPath;
        }

        return d_ptr->loadImage(imagePath);
    }

    std::variant<std::string, ErrorCodes> TGAImageLoader::storeImage(const std::string_view& imagePath, const TGAImage& image)
    {
        if(!verifyDirectoryExistence(imagePath))
        {
            return ErrorCodes::InvalidPath;
        }

        return d_ptr->storeImage(imagePath, image);
    }

    std::variant<std::string, ErrorCodes> TGAImageLoader::storeImage(const std::string_view& imagePath, const TGAImage& image,
                                                                      const compressionStatus& status)
    {
        if(!verifyDirectoryExistence(imagePath))
        {
            return ErrorCodes::InvalidPath;
        }

        if(compressionStatus::NO == status)
        {
            return storeImage(imagePath, image);
        }

        return d_ptr->storeCompressedImage(imagePath, image);
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
