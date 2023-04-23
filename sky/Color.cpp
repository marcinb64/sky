#include "Color.h"

#include <cmath>
#include <moremath.h>

using namespace sky;

SDLColor sky::hsv(float hue, float saturation, float brightness)
{
    hue = hue / 360.0f;
    Uint8 r = 0, g = 0, b = 0;
    if (saturation == 0) {
        r = g = b = (Uint8)lround(brightness * 255.0f);
    } else {
        float h = (hue - floor(hue)) * 6.0f;
        float f = h - floor(h);
        float p = brightness * (1.0f - saturation);
        float q = brightness * (1.0f - saturation * f);
        float t = brightness * (1.0f - (saturation * (1.0f - f)));
        switch ((int)h) {
        case 0:
            r = (Uint8)lround(brightness * 255.0f);
            g = (Uint8)lround(t * 255.0f);
            b = (Uint8)lround(p * 255.0f);
            break;
        case 1:
            r = (Uint8)lround(q * 255.0f);
            g = (Uint8)lround(brightness * 255.0f);
            b = (Uint8)lround(p * 255.0f);
            break;
        case 2:
            r = (Uint8)lround(p * 255.0f);
            g = (Uint8)lround(brightness * 255.0f);
            b = (Uint8)lround(t * 255.0f);
            break;
        case 3:
            r = (Uint8)lround(p * 255.0f);
            g = (Uint8)lround(q * 255.0f);
            b = (Uint8)lround(brightness * 255.0f);
            break;
        case 4:
            r = (Uint8)lround(t * 255.0f);
            g = (Uint8)lround(p * 255.0f);
            b = (Uint8)lround(brightness * 255.0f);
            break;
        case 5:
            r = (Uint8)lround(brightness * 255.0f);
            g = (Uint8)lround(p * 255.0f);
            b = (Uint8)lround(q * 255.0f);
            break;
        }
    }

    return SDLColor {r, g, b, 0xff};
}

/* -------------------------------------------------------------------------- */

HSVRamp::HSVRamp(int numSteps_) : numSteps(numSteps_)
{
}

auto HSVRamp::from(const HSV &color) -> HSVRamp &
{
    startValues = color;
    return *this;
}

auto HSVRamp::to(const HSV &color) -> HSVRamp &
{
    endValues = color;
    return *this;
}

auto HSVRamp::build() const -> void
{
    indexToColor = [&](int i) {
        const auto progress = static_cast<float>(i) / static_cast<float>(numSteps - 1);
        const auto hue = mist::lerp(progress, startValues.hue, endValues.hue);
        const auto sat = mist::lerp(progress, startValues.sat, endValues.sat);
        const auto val = mist::lerp(progress, startValues.val, endValues.val);
        return sky::hsv(hue, sat, val);
    };
}

auto HSVRamp::get(int index) const -> SDLColor
{
    if (!indexToColor) build();
    return indexToColor(index);
}

auto HSVRamp::asFunc() const -> IndexToColor
{
    if (!indexToColor) build();
    return indexToColor;
}
