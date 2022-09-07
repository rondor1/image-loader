#include <iostream>

#include "tgaImage/TGAImageLoad.hpp"

int main(void)
{
    auto loader = new imageloader::TGAImageLoader();

    auto image = loader->loadImage("/home/robert/sample_640×426.tga");

    if(std::holds_alternative<imageloader::TGAImage*>(image))
    {
        auto currentImage = std::get<imageloader::TGAImage*>(image);

        loader->storeImage("sample_640×426.tga", *currentImage);
    }



    delete loader;
    return 0;
}