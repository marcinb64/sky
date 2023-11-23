#ifndef SDLPP_TILES_H_
#define SDLPP_TILES_H_

#include "Color.h"
#include "SkyEngine.h"
#include <mist/Point.h>

#include <functional>
#include <memory>
#include <vector>

namespace sky
{

/**
 * @brief A utility for drawing tiles from a tileset image
 *
 * Holds a collection of tiles in a single image.
 * Provides drawTile() function for drawing a selected tile on target Renderer.
 *
 * Tiles are identified with a single int tileId, counting from 0 in positive X direction,
 * wrapping to the next row after tilesPerRow tiles.
 */
class Tileset
{
public:
    explicit Tileset(const Surface &surface, int tileSize);
    Tileset(const char *file, int tileSize);

    [[nodiscard]] int getTileSize() const { return tileSize; }

    void drawTile(Renderer &renderer, int tileId, SDL_Rect &dest) const;

private:
    Texture texture;
    int     tileSize;
    int     tilesPerRow;
};

using SharedTileset = std::shared_ptr<Tileset>;

// -----------------------------------------------------------------------------

/// @brief A utility for making procedurally generated tilesets
class TilesetBuilder
{
public:
    TilesetBuilder(int tileSize_);

    SharedTileset build();

    /**
     * @brief Tile Painter function for generating tile sequences with addSequence()
     *
     * The painter function will be called for each tile in the sequence.
     * Use the provided renderer to paint each tile at the coordinates given by rect.
     * The index parameter denotes which tile in the sequence is being painted.
     *
     * The index always starts at 0, so if generating tiles form 10 to 15, the
     * index will be from 0 to 5.
     *
     * @arg renderer renderer to use for painting tiles
     * @arg rect coordinates of the tile to paint
     * @arg int index of the tile in sequence
     */
    using TilePainter = std::function<void(Renderer &renderer, SDL_Rect *rect, int index)>;

    TilesetBuilder &addSequence(int count, const TilePainter &painter);
    TilesetBuilder &addTile(const TilePainter &painter);

    void drawTile(int tileId, int painterArg, const TilePainter &painter);

    void exportToTile(const char *pngFile);

private:
    int tileSize;
    int numTiles {0};

    std::unique_ptr<Surface> surface;
    Renderer                 renderer;

    struct Node {
        int         count;
        TilePainter painter;
    };

    std::vector<Node> nodes;
};

/// @brief A collection of tile generation functions for TilesetBuilder
namespace TilePainters
{
using TileToColor = std::function<Color(int)>;

TilesetBuilder::TilePainter plainColor(const TileToColor f);
TilesetBuilder::TilePainter plainColor(const Color &color);
TilesetBuilder::TilePainter plainColor(const HSVRamp &color);

auto hsvValueRamp(float hue, float sat, float fromValue, float toValue, int numSteps)
    -> TilesetBuilder::TilePainter;
} // namespace TilePainters

/* -------------------------------------------------------------------------- */

/// @brief A Drawable that renders a single tile
/// Defined by a Tileset and a tileId from that set.
/// For drawing multiple tiles in a grid, use TileMap
class TileDrawable : public Drawable
{
public:
    TileDrawable(Tileset &tileset_) : tileset(tileset_) {}

    [[nodiscard]] int getTile() const noexcept { return tile; }
    void              setTile(int t) { tile = t; }

    void draw(Renderer &renderer, int x, int y, double) override;

private:
    Tileset &tileset;
    int      tile;
};

/* -------------------------------------------------------------------------- */

/// @brief A Drawable grid of tiles from a single Tileset
class TileMap : public Drawable
{
public:
    TileMap(SharedTileset tileset_, int width_, int height_)
        : tileset(std::move(tileset_)), width(width_), height(height_), tiles(width * height)
    {
    }

    void fill(int x)
    {
        for (auto &i : tiles)
            i = x;
    }

    [[nodiscard]] int getWidth() const noexcept { return width; }

    [[nodiscard]] int getHeight() const noexcept { return height; }

    [[nodiscard]] int getTileSize() const noexcept { return tileset->getTileSize(); }

    [[nodiscard]] int &operator[](const mist::Point2i &p) { return tiles[p.y * width + p.x]; }

    [[nodiscard]] int operator[](const mist::Point2i &p) const { return tiles[p.y * width + p.x]; }

    [[nodiscard]] int &at(const mist::Point2i &p) { return tiles[p.y * width + p.x]; }

    [[nodiscard]] int at(const mist::Point2i &p) const { return tiles[p.y * width + p.x]; }

    void draw(Renderer &renderer, int x, int y, double) override;

private:
    SharedTileset    tileset;
    const int        width;
    const int        height;
    std::vector<int> tiles;
};

using SharedTileMap = std::shared_ptr<TileMap>;

} // namespace sky

#endif
