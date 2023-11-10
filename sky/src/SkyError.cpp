#include "SkyError.h"
#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL2/SDL_ttf.h>

using namespace sky;

SDLError::SDLError() : message(SDL_GetError())
{
}

SDLError::SDLError(const char *m) : message(std::string(m) + ": " + SDL_GetError())
{
}

TTFError::TTFError() : message(TTF_GetError())
{
}

TTFError::TTFError(const char *m) : message(std::string(m) + ": " + TTF_GetError())
{
}

IMGError::IMGError() : message(TTF_GetError())
{
}

IMGError::IMGError(const char *m) : message(std::string(m) + ": " + IMG_GetError())
{
}
