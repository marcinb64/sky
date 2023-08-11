# libsky

## About

2D game dev library based on libSDL, with C++ interface.

## Dependencies

 * [libmist](https://github.com/marcinb64/mist) - game dev utilities
 * SDL, SDL_ttf and SDL_Image dev libraries
 * libpng - dependency of SDL
 * spdlog - logging
 * Catch2 - unit testing

## Build

    mkdir -p build
    conan install --build=missing --output-folder=build conanfile.txt -s build_type=Debug
    cmake --preset conan-debug
    cmake --build build
