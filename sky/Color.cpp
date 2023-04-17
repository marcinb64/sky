#include "Color.h"

#include <cmath>

using namespace sky;


SDLColor sky::hsv(float hue, float saturation, float brightness)
{
    hue = hue / 360.0f;
    Uint8 r = 0, g = 0, b = 0;
    if (saturation == 0) {
        r = g = b = (Uint8)lround(brightness * 255.0f + 0.5f);
    } else {
        float h = (hue - floor(hue)) * 6.0f;
        float f = h - floor(h);
        float p = brightness * (1.0f - saturation);
        float q = brightness * (1.0f - saturation * f);
        float t = brightness * (1.0f - (saturation * (1.0f - f)));
        switch ((int)h) {
        case 0:
            r = (Uint8)lround(brightness * 255.0f + 0.5f);
            g = (Uint8)lround(t * 255.0f + 0.5f);
            b = (Uint8)lround(p * 255.0f + 0.5f);
            break;
        case 1:
            r = (Uint8)lround(q * 255.0f + 0.5f);
            g = (Uint8)lround(brightness * 255.0f + 0.5f);
            b = (Uint8)lround(p * 255.0f + 0.5f);
            break;
        case 2:
            r = (Uint8)lround(p * 255.0f + 0.5f);
            g = (Uint8)lround(brightness * 255.0f + 0.5f);
            b = (Uint8)lround(t * 255.0f + 0.5f);
            break;
        case 3:
            r = (Uint8)lround(p * 255.0f + 0.5f);
            g = (Uint8)lround(q * 255.0f + 0.5f);
            b = (Uint8)lround(brightness * 255.0f + 0.5f);
            break;
        case 4:
            r = (Uint8)lround(t * 255.0f + 0.5f);
            g = (Uint8)lround(p * 255.0f + 0.5f);
            b = (Uint8)lround(brightness * 255.0f + 0.5f);
            break;
        case 5:
            r = (Uint8)lround(brightness * 255.0f + 0.5f);
            g = (Uint8)lround(p * 255.0f + 0.5f);
            b = (Uint8)lround(q * 255.0f + 0.5f);
            break;
        }
    }

    return SDLColor { r, g, b, 0xff };
}
