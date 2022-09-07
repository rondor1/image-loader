set(sources src/tgaImage/TGAImage.cpp
            src/tgaImage/TGAImageLoad.cpp)

set(headers inc/tgaImage/TGAImage.hpp
            inc/tgaImage/Constants.hpp
            inc/tgaImage/TGAImageLoad.hpp
            inc/ErrorCodes.hpp)

add_library(loader ${sources} ${headers})
add_library(${PROJECT_NAME}::loader ALIAS loader)
target_compile_features(loader PUBLIC cxx_std_17)

target_include_directories(loader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)