#ifndef COLOR_H_
#define COLOR_H_

#include <SDL2/SDL.h>
#include <functional>

namespace sky
{

struct SDLColor {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;

    SDL_Color toSdl() const { return SDL_Color {r, g, b, a}; }
};

struct Hue {
    constexpr static auto Red = 0.0f;
    constexpr static auto Yellow = 60.0f;
    constexpr static auto Green = 120.0f;
    constexpr static auto Cyan = 180.0f;
    constexpr static auto Blue = 240.0f;
    constexpr static auto Magenta = 300.0f;
};

struct ColorMask {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    constexpr static Uint32 r = 0xff000000;
    constexpr static Uint32 g = 0x00ff0000;
    constexpr static Uint32 b = 0x0000ff00;
    constexpr static Uint32 a = 0x000000ff;
#else
    constexpr static Uint32 r = 0x000000ff;
    constexpr static Uint32 g = 0x0000ff00;
    constexpr static Uint32 b = 0x00ff0000;
    constexpr static Uint32 a = 0xff000000;
#endif
};

SDLColor hsv(float hue, float saturation, float brightness);

struct HSV {
    float hue;
    float sat;
    float val;
};

class HSVRamp
{
    using IndexToColor = std::function<SDLColor(int)>;

public:
    HSVRamp(int numSteps);
    auto from(const HSV &color) -> HSVRamp &;
    auto to(const HSV &color) -> HSVRamp &;

    auto get(int i) const -> SDLColor;
    auto asFunc() const -> IndexToColor;

private:
    int numSteps;
    HSV startValues;
    HSV endValues;

    mutable IndexToColor indexToColor {nullptr};
    auto                 build() const -> void;
};

} // namespace sky

#endif
