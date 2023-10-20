#include "TileEditor.h"

#include <SDL.h>
#include <spdlog/spdlog.h>

using namespace demo;

TileEditor::TileEditor(sky::Tileset &tileset, size_t width, size_t height)
    : tilemap(std::make_shared<sky::TileMap>(tileset, width, height))
    , tileSize(tileset.getTileSize())
{
    setDrawable(tilemap);
}

bool TileEditor::isValid(const mist::Point2i &p) const
{
    // clang-format off
    return 
        p.x >= 0 &&
        p.y >= 0 &&
        p.x < tilemap->getWidth() &&
        p.y < tilemap->getHeight();
    // clang-format om
}

void TileEditor::brush(const mist::Point2i p, int tile)
{
    if (!isValid(p)) return;
    (*tilemap)[p] = tile;
}

void TileEditor::fill(int tile)
{
    tilemap->fill(tile);
}

/* -------------------------------------------------------------------------- */

TileSelector::TileSelector(std::shared_ptr<demo::TileEditor> editor_, std::shared_ptr<sky::Drawable> drawable_)
    : sky::Object(std::move(drawable_))
    , editor(std::move(editor_))
{
}

void TileSelector::move(const Direction dir)
{
    mist::Point2i n = pointTo(location, dir);
    if (!editor->isValid(n)) return;
    location = n;

    int tileSize = editor->getTileSize();
    setPosition(location.x * tileSize, location.y * tileSize);
}

/* -------------------------------------------------------------------------- */

TilePalette::TilePalette(sky::Tileset &tileset)
{
    drawable = std::make_shared<sky::TileDrawable>(tileset);
    setDrawable(drawable);

    palette.emplace_back(0);
}

void TilePalette::setPalette(const std::vector<int> &tiles)
{
    if (tiles.empty()) return;

    index = 0;
    palette.clear();

    for (auto &i : tiles)
        palette.emplace_back(i);

    drawable->setTile(palette[index]);
}

void TilePalette::selectNext()
{
    index = (index + 1) % palette.size();
    drawable->setTile(palette[index]);
}

void TilePalette::selectPrevious()
{
    index = (index > 0) ? index - 1 : palette.size() - 1;
    drawable->setTile(palette[index]);
}
