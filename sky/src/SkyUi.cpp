#include "SkyUi.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <spdlog/spdlog.h>

using namespace sky;
using namespace mist;



void Widget::draw(Ui &context, Renderer &renderer, int x, int y, double)
{
    if (!valid) {
        texture = render(context);
        valid = true;
    }

    SDL_Rect destRect {x, y, width, height};
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b,
                           backgroundColor.a);
    SDL_RenderFillRect(renderer, &destRect);
    destRect.w = texture.getWidth();
    destRect.h = texture.getHeight();
    texture.renderTo(renderer, nullptr, &destRect);
}

/* -------------------------------------------------------------------------- */

Label::Label(const std::string text_)
    : text(text_)
{
}

Label::Label(const char *text_)
    : text(text_)
{
}

void Label::measure(const Ui &context)
{
    if (font == nullptr) font = context.getDefaultFont();
    Point2i dim = font->measure(text);
    if (width == 0) width = dim.x;
    if (height == 0) height = dim.y;
}

Texture Label::render(Ui &context)
{
    if (font == nullptr) font = context.getDefaultFont();
    return font->renderSolid(text.empty() ? std::string(" "): text, foregroundColor);
}

/* -------------------------------------------------------------------------- */

LinearLayout::LinearLayout(int width_, int height_) noexcept
{
    width = width_;
    height = height_;
}

LinearLayout &LinearLayout::operator+=(UiDrawable *d)
{
    contents.emplace_back(d);
    return *this;
}

void LinearLayout::draw(Ui &context, Renderer &renderer, int x0, int y0, double angle)
{
    int x = x0;
    int y = y0;

    for (auto i : contents) {
        i->draw(context, renderer, x, y, angle);

        if (vertical)
            y += i->height;
        else
            x += i->width;
    }
}

void LinearLayout::measure(const Ui &context)
{
    int w = 0;
    int h = 0;

    for (auto i : contents) {
        i->measure(context);

        if (vertical)
            h += i->height;
        else
            w += i->width;
    }

    width = w;
    height = h;
}

/* -------------------------------------------------------------------------- */

Ui::Ui(std::shared_ptr<Font> defaultFont_)
    : defaultFont(defaultFont_)
{
}

void Ui::measure()
{
    root->measure(*this);
}

void Ui::draw(Renderer &renderer, int x, int y, double angle)
{
    if (root == nullptr) return;
    root->draw(*this, renderer, x, y, angle);
}
