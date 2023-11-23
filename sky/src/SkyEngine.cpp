#include "SkyEngine.h"
#include "Sky.h"

#include <SDL2/SDL.h>

#include <spdlog/spdlog.h>

using namespace sky;

struct LayerId {
    static constexpr auto World = 0;
    static constexpr auto UI = 1;
};

// -----------------------------------------------------------------------------

mist::Point2i Context::worldToScreen(mist::Point2d p)
{
    mist::Point2d screenPoint = worldToScreenTransform(p);
    return mist::round(screenPoint);
}

// -----------------------------------------------------------------------------

EngineScene::EngineScene()
{
}

void EngineScene::setWorldToScreen(const Transform2d &w2s)
{
    layers[LayerId::World].context.setWorldToScreen(w2s);
}

void EngineScene::add(SharedObject o)
{
    layers[LayerId::World].objects.emplace_back(std::move(o));
}

void EngineScene::addUi(SharedObject o)
{
    layers[LayerId::UI].objects.emplace_back(std::move(o));
}

void EngineScene::onDraw(Renderer &renderer)
{
    renderer.setDrawColor(sky::Color {0, 0, 0, 255});
    renderer.clear();

    for (auto &layer : layers) {
        for (auto &o : layer.objects) {
            o->draw(renderer, layer.context);
        }
    }

    onPostDraw(renderer);
    renderer.present();
}

SharedSprite EngineScene::loadSprite(const char *file)
{
    auto tex = std::make_shared<Texture>(Surface::fromFile(file));
    return std::make_shared<Sprite>(tex);
}

// -----------------------------------------------------------------------------

Transform2d Transforms::uiDefault()
{
    return Transform2d::identity();
}

Transform2d Transforms::world(double xExtent, double yExtent, int screenWidth,
                                   int screenHeight)
{
    double ratio = static_cast<double>(screenWidth) / static_cast<double>(screenHeight);
    if (xExtent >= yExtent) {
        yExtent = xExtent / ratio;
    } else {
        xExtent = yExtent * ratio;
    }

    const mist::Point2d world0 = {-xExtent / 2, -yExtent / 2};
    const mist::Point2d world1 = {xExtent / 2, yExtent / 2};
    const mist::Point2d screen0 = {0, static_cast<double>(screenHeight)};
    const mist::Point2d screen1 = {static_cast<double>(screenWidth), 0};

    return {world0, world1, screen0, screen1};
}

Transform2d Transforms::tiles(int tileSize, int screenWidth, int screenHeight)
{
    auto tilesX = static_cast<double>(screenWidth) / tileSize;
    auto tilesY = static_cast<double>(screenHeight) / tileSize;

    const mist::Point2d world0 = {0, 0};
    const mist::Point2d world1 = {tilesX, tilesY};
    const mist::Point2d screen0 = {0, 0};
    const mist::Point2d screen1 = {static_cast<double>(screenWidth),
                                   static_cast<double>(screenHeight)};

    return {world0, world1, screen0, screen1};
}

// -----------------------------------------------------------------------------

Sprite::Sprite(std::shared_ptr<Texture> texture_) : texture(texture_)
{
    width = texture->getWidth();
    height = texture->getHeight();
}

void Sprite::draw(Renderer &renderer, int x, int y, double angle)
{
    SDL_Rect destRect {x - width / 2, y - height / 2, width, height};
    texture->renderTo(renderer, nullptr, &destRect, angle, SDL_FLIP_NONE);
}

// -----------------------------------------------------------------------------

SharedObject Object::from(SharedDrawable d)
{
    return std::make_shared<Object>(std::move(d));
}

Object::Object(SharedDrawable d) : drawable(std::move(d))
{
}

Object &Object::setDrawable(SharedDrawable d)
{
    this->drawable = std::move(d);
    return *this;
}

void Object::draw(Renderer &renderer, Context &context) const
{
    if (drawable == nullptr) return;

    const mist::Point2i screenPos = context.worldToScreen(position);
    drawable->draw(renderer, screenPos.x, screenPos.y, heading);
}
