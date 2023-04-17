#include "sky.h"
#include "Color.h"

#include <SDL.h>
#include <SDL_ttf.h>
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

Sky::Sky()
    : width(0)
    , height(0)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) throw SDLError("SDL_Init");
    if (TTF_Init() < 0) throw TTFError("TTF_Init");
    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) <= 0) throw IMGError("IMG_Init");
}

Sky::~Sky()
{
    if (renderer) SDL_DestroyRenderer(renderer);
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
    activeScene->onLoad();
}

SDL_Renderer *Sky::currentRenderer()
{
    return Sky::getInstance().renderer;
}

void Sky::createWindow(const char *name)
{
    SDL_CreateWindowAndRenderer(width, height,
                                SDL_WINDOW_RESIZABLE, &window, &renderer);
    if (window == nullptr) throw SDLError("Create SDL Window");
    if (renderer == nullptr) throw SDLError("Create SDL Renderer");
    SDL_SetWindowTitle(window, name);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);

    primarySurface = SDL_GetWindowSurface(window);
}

/* -------------------------------------------------------------------------- */

Surface::Surface(SDL_Surface *surface_)
    : surface(surface_)
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

/* -------------------------------------------------------------------------- */

Texture::Texture(SDL_Texture *texture_, int width_, int height_)
    : texture(texture_)
    , width(width_)
    , height(height_)
{
}

Texture::Texture(const Surface &surface)
    : width(surface.getWidth())
    , height(surface.getHeight())
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
    width = other.width;
    height = other.height;
    other.texture = nullptr;
    return *this;
}

void Texture::renderTo(SDL_Renderer *renderer, const SDL_Rect *src, const SDL_Rect *dest,
                       const SDL_RendererFlip flip) const
{
    SDL_RenderCopyEx(renderer, texture, src, dest, 0, nullptr, flip);
}

/* -------------------------------------------------------------------------- */

Sprite::Sprite(const char *file)
{
    auto *tmp = IMG_Load(file);
    if (tmp == nullptr) throw SDLError("Load image");

    width = tmp->w;
    height = tmp->h;
    texture = SDL_CreateTextureFromSurface(Sky::currentRenderer(), tmp);
    SDL_FreeSurface(tmp);
    if (texture == nullptr) throw SDLError("Create image texture");
}

Sprite::~Sprite()
{
    if (texture) SDL_DestroyTexture(texture);
}

void Sprite::draw(SDL_Renderer *renderer, int x, int y, double angle)
{
    SDL_Rect destRect {x, y, width, height};
    SDL_RenderCopyEx(renderer, texture, nullptr, &destRect, angle, nullptr, SDL_FLIP_NONE);
}

/* -------------------------------------------------------------------------- */

std::shared_ptr<Object> Object::from(std::shared_ptr<Drawable> d)
{
    return std::make_shared<Object>(std::move(d));
}

Object::Object(std::shared_ptr<Drawable> d)
    : drawable(std::move(d))
{
}

Object &Object::setDrawable(std::shared_ptr<Drawable> d)
{
    this->drawable = std::move(d);
    return *this;
}

Object &Object::setPosition(int x_, int y_)
{
    this->x = x_;
    this->y = y_;
    return *this;
}

Object &Object::setHeading(double h)
{
    this->heading = h;
    return *this;
}

void Object::draw(SDL_Renderer *renderer) const
{
    if (drawable != nullptr) drawable->draw(renderer, x, y, heading);
}

/* -------------------------------------------------------------------------- */

void Scene::add(std::shared_ptr<Object> o)
{
    renderList.emplace_back(std::move(o));
}

void Scene::draw(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (auto &o : renderList) {
        o->draw(renderer);
    }

    SDL_RenderPresent(renderer);
}

void Scene::update(float dt)
{
    onUpdate(dt);
}

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

/* -------------------------------------------------------------------------- */

SDLError::SDLError()
    : message(SDL_GetError())
{
}
SDLError::SDLError(const char *m)
    : message(std::string(m) + ": " + SDL_GetError())
{
}

TTFError::TTFError()
    : message(TTF_GetError())
{
}

TTFError::TTFError(const char *m)
    : message(std::string(m) + ": " + TTF_GetError())
{
}

IMGError::IMGError()
    : message(TTF_GetError())
{
}

IMGError::IMGError(const char *m)
    : message(std::string(m) + ": " + IMG_GetError())
{
}
