#include "Sky.h"
#include "Color.h"

#include <mist/Point.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL_image.h>
#include <cmath>
#include <memory>
#include <string>

using namespace std;
using namespace sky;

/* -------------------------------------------------------------------------- */

unique_ptr<Sky> Sky::instance = nullptr; // NOLINT

void Sky::initWindow(const char *title, int width, int height)
{
    Sky &sky = Sky::getInstance();
    sky.width = width;
    sky.height = height;
    sky.createWindow(title);
}

Sky &Sky::getInstance()
{
    if (instance.get() == nullptr) instance = std::make_unique<Sky>();
    return *instance;
}

Sky::Sky() : width(0), height(0)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) throw SDLError("SDL_Init");
    if (TTF_Init() < 0) throw TTFError("TTF_Init");
    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) <= 0) throw IMGError("IMG_Init");
}

Sky::~Sky()
{
    if (primarySurface) SDL_FreeSurface(primarySurface);
    if (window) SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    IMG_Quit();
}

void Sky::mainLoop(float fpsCap)
{
    auto minFrameTimeMs = lround(1000.0f / fpsCap);

    auto lastUpdateTime = SDL_GetTicks();
    while (activeScene->isAlive()) {
        auto frameStartTime = SDL_GetTicks();
        activeScene->draw(renderer);
        activeScene->processEvents();

        auto updateTime = SDL_GetTicks();
        auto dt = static_cast<float>(updateTime - lastUpdateTime) / 1000.0f; // NOLINT
        lastUpdateTime = updateTime;
        activeScene->update(dt);

        auto frameEndTime = SDL_GetTicks();
        auto frameTime = frameEndTime - frameStartTime;
        auto timeLeft = minFrameTimeMs - frameTime;
        if (timeLeft > 0) SDL_Delay(static_cast<Uint32>(timeLeft));
    }
}

void Sky::setScene(Scene *scene)
{
    activeScene = scene;
    activeScene->load();
}

SDL_Renderer *Sky::currentRenderer()
{
    return Sky::getInstance().renderer;
}

void Sky::createWindow(const char *name)
{
    SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_RESIZABLE, &window, &renderer.renderer);
    if (window == nullptr) throw SDLError("Create SDL Window");
    if (renderer.renderer == nullptr) throw SDLError("Create SDL Renderer");
    SDL_SetWindowTitle(window, name);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);

    primarySurface = SDL_GetWindowSurface(window);
}

/* -------------------------------------------------------------------------- */

Surface::Surface(SDL_Surface *surface_) : surface(surface_)
{
}

Surface::Surface(int width, int height, int bpp)
{
    using M = sky::ColorMask;
    // NOLINTNEXTLINE
    surface = SDL_CreateRGBSurface(0, width, height, bpp, M::r, M::g, M::b, M::a);
    if (surface == nullptr) throw SDLError("Create surface for tileset");
}

Surface::~Surface()
{
    if (surface) SDL_FreeSurface(surface);
}

Surface::Surface(Surface &&other) noexcept
{
    *this = std::move(other);
}

Surface &Surface::operator=(Surface &&other) noexcept
{
    surface = other.surface;
    other.surface = nullptr;
    return *this;
}

// -----------------------------------------------------------------------------

Renderer::Renderer(SDL_Renderer *r) : renderer(r)
{
}

Renderer::~Renderer()
{
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
}

void Renderer::setDrawColor(const Color &color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void Renderer::clear()
{
    SDL_RenderClear(renderer);
}

void Renderer::present()
{
    SDL_RenderPresent(renderer);
}

// -----------------------------------------------------------------------------

Texture::Texture(SDL_Texture *texture_, int width_, int height_)
    : texture(texture_), width(width_), height(height_)
{
}

Texture::Texture(const Surface &surface) : width(surface.getWidth()), height(surface.getHeight())
{
    texture = SDL_CreateTextureFromSurface(Sky::currentRenderer(), surface.get());
    if (texture == nullptr) throw SDLError("Create texture from surface");
}

Texture::~Texture()
{
    if (texture) SDL_DestroyTexture(texture);
}

Texture::Texture(Texture &&other) noexcept
{
    *this = std::move(other);
}

Texture &Texture::operator=(Texture &&other) noexcept
{
    texture = other.texture;
    other.texture = nullptr;
    width = other.width;
    height = other.height;
    return *this;
}

void Texture::renderTo(Renderer &renderer, const SDL_Rect *src, const SDL_Rect *dest, double angle,
                       const SDL_RendererFlip flip) const
{
    SDL_RenderCopyEx(renderer, texture, src, dest, angle, nullptr, flip);
}

// -----------------------------------------------------------------------------

Font::Font(TTF_Font *font_) : font(font_)
{
}

Font::~Font()
{
    if (font) TTF_CloseFont(font);
}

Texture Font::renderSolid(const std::string &text, const Color &color) const
{
    SDL_Surface *tmp = TTF_RenderText_Solid(font, text.c_str(), color.toSdl());
    if (!tmp) throw TTFError("render text");
    return Texture(Surface(tmp));
}

mist::Point2i Font::measure(const std::string text) const
{
    mist::Point2i ret {0, 0};
    TTF_SizeText(font, text.c_str(), &ret.x, &ret.y);
    return ret;
}

// -----------------------------------------------------------------------------

auto Assets::loadTexture(const char *file) -> std::shared_ptr<Texture>
{
    auto *tmp = IMG_Load(file);
    if (tmp == nullptr) throw SDLError("Load image");

    auto texture = std::make_shared<Texture>(
        SDL_CreateTextureFromSurface(Sky::currentRenderer(), tmp), tmp->w, tmp->h);
    SDL_FreeSurface(tmp);
    if (texture == nullptr) throw SDLError("Create image texture");

    return texture;
}

auto Assets::loadFont(const char *file, int size) -> std::shared_ptr<Font>
{
    auto *font = TTF_OpenFont(file, size);
    if (!font) throw TTFError("open font");
    return std::make_shared<Font>(font);
}

// -----------------------------------------------------------------------------

void Scene::processEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event) != 0) {
        switch (event.type) {
        case SDL_QUIT: alive = false; break;
        case SDL_KEYDOWN: onKeyDown(event.key); break;
        }
    }
}
