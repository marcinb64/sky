#ifndef TILE_EDITOR_H_
#define TILE_EDITOR_H_

#include "Chunk.h"

#include <Sky.h>
#include <Tiles.h>

#include <memory>

struct SDL_KeyboardEvent;

namespace demo
{

class TileEditor : public sky::Object
{
public:
    TileEditor(sky::Tileset &tileset, size_t width, size_t height);

    bool isValid(const mist::Point2i &p) const;
    [[nodiscard]] int getTileSize() const noexcept
    {
        return tileSize;
    }

    [[nodiscard]] int getWidth() const noexcept { return tilemap->getWidth(); }
    [[nodiscard]] int getHeight() const noexcept { return tilemap->getHeight(); }

    void brush(const mist::Point2i p, int tile);

    void fill(int tile);

private:
    std::shared_ptr<sky::TileMap> tilemap;
    int tileSize;
};

/* -------------------------------------------------------------------------- */

class TileSelector : public sky::Object
{
public:
    TileSelector(std::shared_ptr<demo::TileEditor> editor, std::shared_ptr<sky::Drawable> drawable);

    [[nodiscard]] const mist::Point2i &getLocation() const noexcept
    {
        return location;
    }

    void move(const Direction dir);

private:
    std::shared_ptr<demo::TileEditor> editor;
    mist::Point2i location {0, 0};
};

/* -------------------------------------------------------------------------- */

class TilePalette : public sky::Object
{
public:
    TilePalette(sky::Tileset &tileset);

    void setPalette(const std::vector<int> &tiles);

    [[nodiscard]] int getSelectedTile() const noexcept { return palette[index]; }
    void selectNext();
    void selectPrevious();

private:
    std::shared_ptr<sky::TileDrawable> drawable;
    std::vector<int> palette;
    size_t index { 0 };
};

/* -------------------------------------------------------------------------- */

} // namespace demo

#endif
