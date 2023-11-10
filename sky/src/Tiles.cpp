#include "Tiles.h"
#include "Color.h"
#include <mist/moremath.h>

#include <SDL_image.h>
#include <spdlog/spdlog.h>
#include <cassert>

using namespace sky;

/* -------------------------------------------------------------------------- */

Tileset::Tileset(const char *file, int tileSize_) : tileSize(tileSize_)
{
    auto *tmp = IMG_Load(file);
    if (tmp == nullptr) throw SDLError("Load tileset image");
    Surface surface {tmp};

    auto width = surface.getWidth();
    tilesPerRow = width / tileSize;

    assert(tilesPerRow > 0); // NOLINT

    texture = Texture {surface};
}

Tileset::Tileset(const Surface &surface, int tileSize_)
    : texture(surface), tileSize(tileSize_), tilesPerRow(surface.getWidth() / tileSize)
{
}

void Tileset::drawTile(Renderer &renderer, int tileId, SDL_Rect &dest) const
{
    auto     row = tileId / tilesPerRow;
    auto     col = tileId % tilesPerRow;
    SDL_Rect src {col * tileSize, row * tileSize, tileSize, tileSize};
    texture.renderTo(renderer, &src, &dest);
}

/* -------------------------------------------------------------------------- */

TilesetBuilder::TilesetBuilder(int tileSize_) : tileSize(tileSize_)
{
}

TilesetBuilder &TilesetBuilder::addSequence(int count, const TilePainter &painter)
{
    numTiles += count;
    nodes.emplace_back(Node {count, painter});

    return *this;
}

TilesetBuilder &TilesetBuilder::addTile(const TilePainter &painter)
{
    addSequence(1, painter);
    return *this;
}

Tileset TilesetBuilder::build()
{
    surface = std::make_unique<Surface>(tileSize * numTiles, tileSize, 32);
    renderer = Renderer(SDL_CreateSoftwareRenderer(surface->get()));
    if (renderer == nullptr) throw SDLError("Create software renderer");

    renderer.setDrawColor(Color { 0, 0, 0, 255});
    renderer.clear();

    int startingTile = 0;
    for (const auto &node : nodes) {
        for (int i = 0; i < node.count; i++) {
            SDL_Rect rect {(startingTile + i) * tileSize, 0, tileSize, tileSize};
            node.painter(renderer, &rect, i);
        }
        startingTile += node.count;
    }

    return Tileset(*surface, tileSize);
}

Tileset TilesetBuilder::update()
{
    return Tileset(*surface, tileSize);
}


auto TilesetBuilder::drawTile(int tileId, int painterArg, const TilePainter &painter) -> void
{
    if (renderer == nullptr) throw SDLError("Tileset not yet built");
    //SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 0xFF);
    //SDL_RenderClear(renderer.get());

    SDL_Rect rect {tileId * tileSize, 0, tileSize, tileSize};
    painter(renderer, &rect, painterArg);
}

/* -------------------------------------------------------------------------- */

TilesetBuilder::TilePainter TilePainters::plainColor(const TileToColor f)
{
    return [=](Renderer &renderer, SDL_Rect *rect, int tile) {
        auto color = f(tile);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xFF);
        SDL_RenderFillRect(renderer, rect);
    };
}

TilesetBuilder::TilePainter TilePainters::plainColor(const Color &color)
{
    return [=](Renderer &renderer, SDL_Rect *rect, int) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xFF);
        SDL_RenderFillRect(renderer, rect);
    };
}

TilesetBuilder::TilePainter TilePainters::plainColor(const HSVRamp &ramp)
{
    return [=](Renderer &renderer, SDL_Rect *rect, int tile) {
        const auto color = ramp.get(tile);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xFF);
        SDL_RenderFillRect(renderer, rect);
    };
}

auto TilePainters::hsvValueRamp(float hue, float sat, float fromValue, float toValue, int numSteps)
    -> TilesetBuilder::TilePainter
{
    return plainColor([=](int i) {
        const auto val = mist::lerp((static_cast<float>(i) / static_cast<float>(numSteps - 1)),
                                    fromValue, toValue);
        return sky::hsv(hue, sat, val);
    });
}

/* -------------------------------------------------------------------------- */

void TileDrawable::draw(Renderer &renderer, int x, int y, double)
{
    auto     tileSize = tileset.getTileSize();
    SDL_Rect destRect {x, y, tileSize, tileSize};
    tileset.drawTile(renderer, tile, destRect);
}

/* -------------------------------------------------------------------------- */

void TileMap::draw(Renderer &renderer, int x, int y, double)
{
    auto     tileSize = tileset.getTileSize();
    SDL_Rect destRect {x, y, tileSize, tileSize};

    for (int iy = 0; iy < height; iy++) {
        for (int ix = 0; ix < width; ix++) {
            destRect.x = x + tileSize * ix;
            destRect.y = y + tileSize * iy;
            tileset.drawTile(renderer, tiles[iy * width + ix], destRect);
        }
    }
}
