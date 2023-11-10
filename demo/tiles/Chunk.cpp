#include "Chunk.h"

using namespace demo;
using namespace mist;

namespace demo
{
Point2i pointTo(const Point2i &p, Direction dir)
{
    switch (dir) {
    case North: return Point2i {p.x, p.y - 1};
    case South: return Point2i {p.x, p.y + 1};
    case West: return Point2i {p.x - 1, p.y};
    case East: return Point2i {p.x + 1, p.y};
    default: return p;
    }
}

Direction opposite(Direction dir)
{
    switch (dir) {
    case North: return South;
    case South: return North;
    case East: return West;
    case West: return East;
    }
    return North;
}
}

/* -------------------------------------------------------------------------- */

void TileRules::allowTransitions(int fromTile, int toTile, Direction dir)
{
    tiles[fromTile].allowedTransitions[dir].emplace(toTile);
    tiles[toTile].allowedTransitions[opposite(dir)].emplace(fromTile);
}

void TileRules::addTransitionSequence(int inner, int outter, std::array<int, 9> tt)
{
    auto nw = tt[0];
    auto n = tt[1];
    auto ne = tt[2];

    auto w = tt[3];
    auto e = tt[5];

    auto sw = tt[6];
    auto s = tt[7];
    auto se = tt[8];

    // inner <-> inner, all dirs
    allowTransitions(inner, inner, North);
    allowTransitions(inner, inner, South);
    allowTransitions(inner, inner, West);
    allowTransitions(inner, inner, East);

    // outter <-> outter, all dirs
    allowTransitions(outter, outter, North);
    allowTransitions(outter, outter, South);
    allowTransitions(outter, outter, West);
    allowTransitions(outter, outter, East);

    // inner <-> cardinal dirs
    allowTransitions(inner, n, North);
    allowTransitions(n, outter, North);
    allowTransitions(inner, s, South);
    allowTransitions(s, outter, South);
    allowTransitions(inner, w, West);
    allowTransitions(w, outter, West);
    allowTransitions(inner, e, East);
    allowTransitions(e, outter, East);

    // outter <-> cardinal dirs
    allowTransitions(outter, nw, South);
    allowTransitions(outter, n, South);
    allowTransitions(outter, ne, South);
    allowTransitions(outter, sw, North);
    allowTransitions(outter, s, North);
    allowTransitions(outter, se, North);
    allowTransitions(outter, nw, East);
    allowTransitions(outter, w, East);
    allowTransitions(outter, sw, East);
    allowTransitions(outter, ne, West);
    allowTransitions(outter, e, West);
    allowTransitions(outter, se, West);

    // between transitive tiles
    allowTransitions(n, nw, West);
    allowTransitions(n, ne, East);
    allowTransitions(s, sw, West);
    allowTransitions(s, se, East);
    allowTransitions(w, nw, North);
    allowTransitions(w, sw, South);
    allowTransitions(e, ne, North);
    allowTransitions(e, se, South);

    // cardinal dir transition tiles in a row
    allowTransitions(n, n, East);
    allowTransitions(s, s, East);
    allowTransitions(e, e, North);
    allowTransitions(w, w, North);

    // cardinal dir transition tiles next to each other
    allowTransitions(n, s, North);
    allowTransitions(w, e, West);

    // corner pieces next to each other
    allowTransitions(nw, ne, East);
    allowTransitions(nw, ne, West);
    allowTransitions(sw, se, East);
    allowTransitions(sw, se, West);
    allowTransitions(nw, sw, South);
    allowTransitions(nw, sw, North);
    allowTransitions(ne, se, South);
    allowTransitions(ne, se, North);
}

std::set<int> TileRules::getAllowedTransitions(int from, Direction dir)
{
    std::set<int> ret;
    for (const auto &i : tiles[from].allowedTransitions[dir])
        ret.emplace(i);

    return ret;
}

bool TileRules::isTransitionAllowed(int from, int to, Direction dir)
{
    auto &t = tiles[from].allowedTransitions[dir];
    return t.find(to) != t.end();
}

void TileRules::addTileFromSubtiles(int tile, std::array<int, 4> subtiles)
{
    subs.emplace_back(Subtile {tile, subtiles});
    tiles[tile] = Tile {};
}

void TileRules::buildTransitionsFromSubtiles()
{
    for (Subtile &from : subs) {
        for (Subtile &to : subs) {
            buildTransitionsBetween(from, to);
        }
    }
}

void TileRules::buildTransitionsBetween(const Subtile &from, const Subtile &to)
{
    if (from.ne() == to.nw() && from.se() == to.sw()) allowTransitions(from.tile, to.tile, East);
    if (from.nw() == to.ne() && from.sw() == to.se()) allowTransitions(from.tile, to.tile, West);
    if (from.ne() == to.se() && from.nw() == to.sw()) allowTransitions(from.tile, to.tile, North);
    if (from.se() == to.ne() && from.sw() == to.nw()) allowTransitions(from.tile, to.tile, South);
}

/* -------------------------------------------------------------------------- */

class SampleTilesetBuilder
{
public:
    SampleTilesetBuilder(TileRules &rules_, int stride_) : rules(rules_), stride(stride_) {}

    auto add(int t)
    {
        rules.addTileFromSubtiles(t, {t, t, t, t});
        return *this;
    }

    auto transitionPattern(int o, int x, mist::Point2i start)
    {
        // xx xx xx
        // xo oo ox
        rules.addTileFromSubtiles(index(start, {0, 0}), {x, x, x, o});
        rules.addTileFromSubtiles(index(start, {1, 0}), {x, x, o, o});
        rules.addTileFromSubtiles(index(start, {2, 0}), {x, x, o, x});

        // xo    ox
        // xo    ox
        rules.addTileFromSubtiles(index(start, {0, 1}), {x, o, x, o});
        rules.addTileFromSubtiles(index(start, {2, 1}), {o, x, o, x});

        // xo oo ox
        // xx xx xx
        rules.addTileFromSubtiles(index(start, {0, 2}), {x, o, x, x});
        rules.addTileFromSubtiles(index(start, {1, 2}), {o, o, x, x});
        rules.addTileFromSubtiles(index(start, {2, 2}), {o, x, x, x});

        // oo oo
        // ox xo
        // ox xo
        // oo oo
        rules.addTileFromSubtiles(index(start, {3, 0}), {o, o, o, x});
        rules.addTileFromSubtiles(index(start, {4, 0}), {o, o, x, o});
        rules.addTileFromSubtiles(index(start, {3, 1}), {o, x, o, o});
        rules.addTileFromSubtiles(index(start, {4, 1}), {x, o, o, o});

        // ox xo
        // xo ox
        rules.addTileFromSubtiles(index(start, {3, 2}), {o, x, x, o});
        rules.addTileFromSubtiles(index(start, {4, 2}), {x, o, o, x});

        return *this;
    }

    void build() { rules.buildTransitionsFromSubtiles(); }

private:
    TileRules &rules;
    int        stride;

    int index(const mist::Point2i &p, const mist::Point2i &offset)
    {
        return (p.y + offset.y) * stride + (p.x + offset.x);
    }
};
