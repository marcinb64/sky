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

Example build commands, using conan profiles `linux-debug` and `linux-release`.

    conan install conanfile.py --build=missing --profile linux-debug
    conan install conanfile.py --build=missing --profile linux-release
    cmake --preset conan-debug
    cmake --build build/Debug

    conan create . --profile=linux-release



