#include <iostream>

#include "tgaImage/TGAImageLoad.hpp"
#include "Logger.hpp"

int main(int argc, char** argv)
{

    if(argc <= 1 || argv == nullptr)
    {
        std::cerr << "Invalid startup of application"<<std::endl;
    }

    utils::logger::setup(std::string_view{utils::constants::info});

    utils::logger::infoMessage("Starting application...");

    auto loader = new imageloader::TGAImageLoader();

    auto image = loader->loadImage(std::string_view{argv[1]});

    if(std::holds_alternative<imageloader::TGAImage*>(image))
    {
        auto currentImage = std::get<imageloader::TGAImage*>(image);

        loader->storeImage(std::string_view{argv[2]}, *currentImage);
    }



    delete loader;
    return 0;
}