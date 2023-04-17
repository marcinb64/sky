#include "tiles.h"
#include "Color.h"

#include <SDL_image.h>
#include <cassert>
#include <moremath.h>
#include <spdlog/spdlog.h>

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

void Tileset::drawTile(SDL_Renderer *renderer, int tileId, SDL_Rect &dest) const
{
    auto     row = tileId / tilesPerRow;
    auto     col = tileId % tilesPerRow;
    SDL_Rect src {col * tileSize, row * tileSize, tileSize, tileSize};
    texture.renderTo(renderer, &src, &dest);
}

/* -------------------------------------------------------------------------- */

TilesetBuilder::TilesetBuilder(int tileSize_, int numTiles_)
    : tileSize(tileSize_), numTiles(numTiles_), surface(tileSize * numTiles, tileSize, 32),
      renderer(SDL_CreateSoftwareRenderer(surface.get()))
{
    if (renderer == nullptr) throw SDLError("Create software renderer");

    SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 0xFF);
    SDL_RenderClear(renderer.get());
}

Tileset TilesetBuilder::build()
{
    return Tileset(surface, tileSize);
}

TilesetBuilder &TilesetBuilder::generateSequence(int startingTile, int count,
                                                 const TilePainter painter)
{
    for (int i = 0; i < count; i++) {
        SDL_Rect rect {(startingTile + i) * tileSize, 0, tileSize, tileSize};
        painter(renderer.get(), &rect, i);
    }

    return *this;
}

TilesetBuilder &TilesetBuilder::generateTile(int tile, const TilePainter painter)
{
    SDL_Rect rect {tile * tileSize, 0, tileSize, tileSize};
    painter(renderer.get(), &rect, tile);
    return *this;
}

/* -------------------------------------------------------------------------- */

TilesetBuilder::TilePainter TilePainters::plainColor(const TileToColor f)
{
    return [=](SDL_Renderer *renderer, SDL_Rect *rect, int tile) {
        auto color = f(tile);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xFF);
        SDL_RenderFillRect(renderer, rect);
    };
}

TilesetBuilder::TilePainter TilePainters::plainColor(const SDLColor color)
{
    return [=](SDL_Renderer *renderer, SDL_Rect *rect, int) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xFF);
        SDL_RenderFillRect(renderer, rect);
    };
}

auto TilePainters::hsvValueRamp(float hue, float sat, float fromValue, float toValue, int numSteps)
    -> TilesetBuilder::TilePainter
{
    return plainColor([=](int i) {
        const auto val =
            mist::lerp((static_cast<float>(i) / static_cast<float>(numSteps)), fromValue, toValue);
        return sky::hsv(hue, sat, val);
    });
}

/* -------------------------------------------------------------------------- */

void TileDrawable::draw(SDL_Renderer *renderer, int x, int y, double)
{
    auto     tileSize = tileset.getTileSize();
    SDL_Rect destRect {x, y, tileSize, tileSize};
    tileset.drawTile(renderer, tile, destRect);
}

/* -------------------------------------------------------------------------- */

void TileMap::draw(SDL_Renderer *renderer, int x, int y, double)
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
