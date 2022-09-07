set(sources src/Logger.cpp)
set(headers inc/Logger.hpp
            inc/Constants.hpp)


add_library(utils ${headers} ${sources})
add_library(${PROJECT_NAME}::utils ALIAS utils)
target_compile_features(utils PUBLIC cxx_std_17)

target_link_libraries(utils PUBLIC CONAN_PKG::spdlog)
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)