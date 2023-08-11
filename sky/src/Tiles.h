#ifndef SDLPP_TILES_H_
#define SDLPP_TILES_H_

#include "Color.h"
#include "Sky.h"
#include <mist/Point.h>

#include <functional>
#include <memory>
#include <vector>

namespace sky
{

class Tileset
{
public:
    explicit Tileset(const Surface &surface, int tileSize);
    Tileset(const char *file, int tileSize);

    [[nodiscard]] int getTileSize() const { return tileSize; }

    void drawTile(SDL_Renderer *renderer, int tileId, SDL_Rect &dest) const;

private:
    Texture texture;
    int     tileSize;
    int     tilesPerRow;
};

/* -------------------------------------------------------------------------- */

class TilesetBuilder
{
public:
    TilesetBuilder(int tileSize_);

    Tileset build();
    Tileset update();

    /**
     * @brief Tile Painer function for generating tile sequences with generateSequence()
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
    using TilePainter = std::function<void(SDL_Renderer *renderer, SDL_Rect *rect, int index)>;

    TilesetBuilder &addSequence(int count, const TilePainter &painter);
    TilesetBuilder &addTile(const TilePainter &painter);

    auto getRenderer() { return renderer.get(); }
    auto drawTile(int tileId, int painterArg, const TilePainter &painter) -> void;

private:
    int tileSize;
    int numTiles {0};

    std::unique_ptr<Surface> surface;
    std::unique_ptr<SDL_Renderer, RendererDeleter> renderer;

    struct Node {
        int         count;
        TilePainter painter;
    };

    std::vector<Node> nodes;
};

namespace TilePainters
{
using TileToColor = std::function<SDLColor(int)>;

TilesetBuilder::TilePainter plainColor(const TileToColor f);
TilesetBuilder::TilePainter plainColor(const SDLColor &color);
TilesetBuilder::TilePainter plainColor(const HSVRamp &color);
auto hsvValueRamp(float hue, float sat, float fromValue, float toValue, int numSteps)
    -> TilesetBuilder::TilePainter;
} // namespace TilePainters

/* -------------------------------------------------------------------------- */

class TileDrawable : public Drawable
{
public:
    TileDrawable(Tileset &tileset_) : tileset(tileset_) {}

    [[nodiscard]] int getTile() const noexcept { return tile; }
    void              setTile(int t) { tile = t; }

    void draw(SDL_Renderer *renderer, int x, int y, double) override;

private:
    Tileset &tileset;
    int      tile;
};

/* -------------------------------------------------------------------------- */

class TileMap : public Drawable
{
public:
    TileMap(const Tileset &tileset_, int width_, int height_)
        : tileset(tileset_), width(width_), height(height_), tiles(width * height)
    {
    }

    void fill(int x)
    {
        for (auto &i : tiles)
            i = x;
    }

    [[nodiscard]] int getWidth() const noexcept { return width; }

    [[nodiscard]] int getHeight() const noexcept { return height; }

    [[nodiscard]] int getTileSize() const noexcept { return tileset.getTileSize(); }

    [[nodiscard]] int &operator[](const mist::Point2i &p) { return tiles[p.y * width + p.x]; }

    [[nodiscard]] int operator[](const mist::Point2i &p) const { return tiles[p.y * width + p.x]; }

    void draw(SDL_Renderer *renderer, int x, int y, double) override;

private:
    const Tileset   &tileset;
    const int        width;
    const int        height;
    std::vector<int> tiles;
};

} // namespace sky

#endif
