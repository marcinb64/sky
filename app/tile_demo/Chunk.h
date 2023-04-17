#ifndef CHUNK_H_
#define CHUNK_H_

#include <Point.h>

#include <array>
#include <fmt/format.h>
#include <initializer_list>
#include <map>
#include <set>
#include <vector>
#include <sstream>

namespace demo
{

enum Direction { North, West, South, East };

mist::Point2i pointTo(const mist::Point2i &p, Direction dir);
Direction opposite(Direction dir);

/* -------------------------------------------------------------------------- */

struct Tile {
    std::array<std::set<int>, 4> allowedTransitions;
};

/* -------------------------------------------------------------------------- */

struct Subtile
{
    int tile;
    std::array<int, 4> subtiles;

    int nw() const { return subtiles[0]; }
    int ne() const { return subtiles[1]; }
    int sw() const { return subtiles[2]; }
    int se() const { return subtiles[3]; }
};

class TileRules
{
public:
    void allowTransitions(int fromTile, int toTile, Direction dir);
    void addTransitionSequence(int inner, int outter, std::array<int, 9> tt);
    std::set<int> getAllowedTransitions(int from, Direction dir);
    bool isTransitionAllowed(int from, int to, Direction dir);

    void addTileFromSubtiles(int tile, std::array<int, 4> subtiles);
    void buildTransitionsFromSubtiles();
    void buildTransitionsBetween(const Subtile &from, const Subtile &to);
private:
    std::map<int, Tile> tiles;

    std::vector<Subtile> subs;
};

/* -------------------------------------------------------------------------- */

struct Cell {
    int terrainTile;
};

/* -------------------------------------------------------------------------- */

constexpr auto CHUNK_SIZE = 16;

class MapChunk
{
public:
    [[nodiscard]] Cell &operator[](const mist::Point2i &p)
    {
        return cells[p.x][p.y];
    }

    [[nodiscard]] const Cell &operator[](const mist::Point2i &p) const
    {
        return cells[p.x][p.y];
    }

    [[nodiscard]] size_t getWidth() const
    {
        return width;
    }

    [[nodiscard]] size_t getHeight() const
    {
        return height;
    }

    bool isValid(const mist::Point2i &p) const
    {
        return p.x >= 0 && p.x < width && p.y >= 0 &&
               p.y < height;
    }

    void fillTerrain(int tile)
    {
        for (auto &i : cells)
            for (auto &j : i)
                j.terrainTile = tile;
    }

private:
    int width = CHUNK_SIZE;
    int height = CHUNK_SIZE;
    std::array<std::array<Cell, CHUNK_SIZE>, CHUNK_SIZE> cells{};
};

}

template <>
struct fmt::formatter<mist::Point2i> {
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin())
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const mist::Point2i &c, FormatContext &ctx) -> decltype(ctx.out())
    {
        return format_to(ctx.out(), "({}, {})", c.x, c.y);
    }
};

template <>
struct fmt::formatter<std::set<int>> {
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin())
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const std::set<int> &arg, FormatContext &ctx) -> decltype(ctx.out())
    {
        std::stringstream ss;
        for (auto &i : arg) ss << i << ",";
        return format_to(ctx.out(), "[{}]", ss.str());
    }
};


#endif
