#include "SkyEngine.h"
#include "Sky.h"

#include <SDL2/SDL.h>

using namespace sky;


void EngineScene::add(std::shared_ptr<Object> o)
{
    renderList.emplace_back(std::move(o));
}

void EngineScene::onDraw(Renderer &renderer)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (auto &o : renderList) {
        o->draw(renderer);
    }

    SDL_RenderPresent(renderer);
}

// -----------------------------------------------------------------------------

Sprite::Sprite(std::shared_ptr<Texture> texture_) : texture(texture_)
{
    width = texture->getWidth();
    height = texture->getHeight();
}

void Sprite::draw(Renderer &renderer, int x, int y, double angle)
{
    SDL_Rect destRect {x, y, width, height};
    texture->renderTo(renderer, nullptr, &destRect, angle, SDL_FLIP_NONE);
}

// -----------------------------------------------------------------------------

std::shared_ptr<Object> Object::from(std::shared_ptr<Drawable> d)
{
    return std::make_shared<Object>(std::move(d));
}

Object::Object(std::shared_ptr<Drawable> d) : drawable(std::move(d))
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

void Object::draw(Renderer &renderer) const
{
    if (drawable != nullptr) drawable->draw(renderer, x, y, heading);
}
