#include "tgaImage/TGAImageLoad.hpp"
#include "Logger.hpp"

int main(int argc, char** argv)
{

    utils::logger::setup(utils::constants::info);

    if(argc <= 1 || argv == nullptr)
    {
        utils::logger::criticalMessage("Invalid startup of the application! Please provide input and output parameter!");
        return -1;
    }

    utils::logger::infoMessage("Initializing image loader...");
    std::unique_ptr imagePointer = std::make_unique<imageloader::TGAImageLoader>();

    utils::logger::infoMessage("Loading image with provided path: " + std::string{argv[1]});
    auto loadResult = imagePointer->loadImage(argv[1]);

    if(std::holds_alternative<imageloader::ErrorCodes>(loadResult))
    {
        auto err = std::get<imageloader::ErrorCodes>(loadResult);
        if(err == imageloader::ErrorCodes::InvalidPath)
        {
            utils::logger::setup(utils::constants::error);
            utils::logger::errorMessage("Invalid path provided!");
            return -1;
        }

        if(err == imageloader::ErrorCodes::InvalidReadOperation)
        {
            utils::logger::setup(utils::constants::error);
            utils::logger::errorMessage("Read operation failed!");
            return -1;
        }
    }
    else
    {
        utils::logger::setup("info");
        std::unique_ptr<imageloader::TGAImage> image{std::get<imageloader::TGAImage*>(loadResult)};
        if(image)
        {
            utils::logger::infoMessage("Image successfully created!");
        }

        auto result = imagePointer->storeImage(argv[2], *(image.get()));

        if(std::holds_alternative<imageloader::ErrorCodes>(result))
        {
            auto err = std::get<imageloader::ErrorCodes>(result);
            if(err == imageloader::ErrorCodes::InvalidPath)
            {
                utils::logger::setup(utils::constants::error);
                utils::logger::errorMessage("Invalid path provided!");
                return -1;
            }

            if(err == imageloader::ErrorCodes::InvalidReadOperation)
            {
                utils::logger::setup(utils::constants::error);
                utils::logger::errorMessage("Read operation failed!");
                return -1;
            }
        }
        else
        {
            auto storePath = std::get<std::string>(result);
             utils::logger::infoMessage("Image successfully stored in: " + storePath);
        }
    }

    return 0;
}