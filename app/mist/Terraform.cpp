#include "Terraform.h"

#include <MapTools.h>
#include <Noise.h>
#include <moremath.h>

#include <random>

using namespace demo;
using Point2i = mist::Point2i;

Terraformer::Terraformer(const mist::Size &size, TileProvider &tileProvider_)
    : tileMap(size), groundMap(size), biomeMap(size), tileProvider(tileProvider_), router(groundMap)
{
    router.setWaterSettings(waterSettings);
}

auto Terraformer::setSeed(long seed) -> Terraformer &
{
    mist::setPerlinSeed(seed);
    return *this;
}

auto Terraformer::setNoiseSettings(const NoiseSettings &settings) -> Terraformer &
{
    noiseSettings = settings;
    return *this;
}

auto Terraformer::setWaterSettings(const WaterSettings &settings) -> Terraformer &
{
    waterSettings = settings;
    return *this;
}

auto Terraformer::addBiome(int biome, double endValue) -> Terraformer &
{
    biomes.emplace_back(BiomeSelector(lastBiomeEndValue, endValue, biome));
    lastBiomeEndValue = endValue;
    return *this;
}

auto Terraformer::generate() -> void
{
    groundMap.fill(0);
    biomeMap.fill(0);

    mist::PerlinNoise2 perlin;
    mist::OctaveNoise2 octaves = mist::OctaveNoise2(perlin)
                                     .setNumOctaves(noiseSettings.numOctaves)
                                     .setRoughness(noiseSettings.roughness)
                                     .setFrequencyMultiplier(noiseSettings.frequencyMultiplier);
    mist::DomainWarpedNoise2 warp(octaves);

    mist::NoiseTextureBuilder2(groundMap, warp)
        .setNoiseScale(noiseSettings.noiseScale)
        .setXScale(noiseSettings.xyScale)
        .setYScale(noiseSettings.xyScale)
        .build();

    mist::LinearTransform dirtValues(-1.0, 0.0, 0.0, 1.0);
    mist::LinearTransform grassValues(0.0, 1.0, 0.0, 1.0);

    auto tilePainter = [&](const mist::Point2i &p, double v) {
        for (const auto &i : biomes) {
            if (v < i.endValue) {
                tileMap.at(p) = tileProvider.getBiomeTile(i.biome, i.noiseToValue(v));
                return;
            }
        }
    };

    groundMap.foreachKeyValue(tilePainter);

    if (waterSettings.biome >= 0) makeRiver();
}

void Terraformer::makeRiver()
{
    const auto route = router.findRiverRoute(static_cast<float>(groundMap.getSize().x) / 2.0f);
    if (!route.empty()) {
        const auto bedLevel = (groundMap.at(route.front()) + groundMap.at(route.back())) / 2;

        makeRiverAlong(route, mist::roundi(waterSettings.riverMaxSize / 2), bedLevel,
                       bedLevel + waterSettings.riverDepth);
    }
}

auto Terraformer::makeRiverAlong(const std::list<mist::Point2i> &route, double maxDistance,
                                 double riverbedLevel, double waterLevel) -> void
{
    router.digTrenchAlong(route, mist::roundi(maxDistance / 2), riverbedLevel);

    const mist::LinearTransform groundToWater(riverbedLevel, waterLevel, 0.0, 1.0);

    for (const auto &p0 : route) {
        mist::floodFill(groundMap, p0, maxDistance, waterLevel, [&](const Point2i &p) {
            auto g = std::clamp(groundMap.at(p), riverbedLevel, waterLevel);
            tileMap.at(p) = tileProvider.getBiomeTile(waterSettings.biome, groundToWater(g));
        });
    }
}

/* -------------------------------------------------------------------------- */

auto RiverRouter::setWaterSettings(const WaterSettings &settings_) -> RiverRouter &
{
    settings = settings_;
    return *this;
}

auto RiverRouter::findRiverRoute(float minDistance) -> std::list<Point2i>
{
    // Find low points along all edges of the map
    std::vector<Point2i> candidates;

    // Scan for end point candidates along the 4 edges of the map
    // Look for low points that are far enough from each other
    auto xSpacing = groundMap.getXSize() / 8;
    auto ySpacing = groundMap.getYSize() / 8;
    auto tmp = findEndpointCandidates({0, 0}, {0, 1}, -0.25, ySpacing);
    candidates.insert(candidates.end(), tmp.begin(), tmp.end());
    tmp = findEndpointCandidates({groundMap.getXSize() - 1, 0}, {0, 1}, -0.25, ySpacing);
    candidates.insert(candidates.end(), tmp.begin(), tmp.end());
    tmp = findEndpointCandidates({0, 0}, {1, 0}, -0.25, xSpacing);
    candidates.insert(candidates.end(), tmp.begin(), tmp.end());
    tmp = findEndpointCandidates({0, groundMap.getYSize() - 1}, {1, 0}, -0.25, xSpacing);
    candidates.insert(candidates.end(), tmp.begin(), tmp.end());

    // Need at least start and end point
    if (candidates.size() < 2) return {};

    for (const auto &start : candidates) {
        // Build a map of the cost to reach each place from the start point
        mist::AStar router(groundMap);
        router
            .setStepCostFactor(settings.routeCost) // cost of going to higher ground
            .setBlockValue(0.2) // Terrain height is [-1, 1], don't allow river above 0.0
            .calculate(start);

        // Consider endpoints in order of descending distance from start
        std::vector<Point2i> endCandidates = candidates;
        std::sort(endCandidates.begin(), endCandidates.end(),
                  [&](const Point2i &a, const Point2i &b) {
                      return (a - start).length() > (b - start).length();
                  });

        // Try to find a route
        for (const auto &end : endCandidates) {
            if (start == end) continue;
            if ((start - end).length() < minDistance) continue; // require minumum distance
            if (start.x == end.x || start.y == end.y) continue; // skip straight lines

            if (!router.canReach(end)) continue;

            return router.route(end);
        }
    }

    return {};
}

auto RiverRouter::findEndpointCandidates(const Point2i &p0, const Point2i &step, double maxLevel,
                                         int minDistance) -> std::vector<Point2i>
{
    std::vector<Point2i> ret;
    const Point2i        spacer {step.x * minDistance, step.y * minDistance};

    for (Point2i p = p0; groundMap.contains(p); p += step) {
        // find suitably low ground
        if (groundMap.at(p) <= maxLevel) {
            // find where the low ground ends
            const auto regionStart = p;
            while (groundMap.contains(p) && groundMap.at(p) <= maxLevel)
                p += step;

            // add the middle point to candidate list
            const auto regionCenter = regionStart + (p - regionStart) / 2;
            ret.emplace_back(regionCenter);

            // skip ahead to guarantee min distance between candidates
            p += spacer;
        }
    }

    return ret;
}

auto RiverRouter::digTrenchAlong(const std::list<mist::Point2i> &route, int radius, double level)
    -> void
{
    mist::MapBrush mapBrush(groundMap, radius);

    mapBrush.atPoints(route, [&](const mist::Point2i &p, double r) {
        const auto bed = std::min(groundMap.at(p), level);
        // brush strength falls off with distance squared
        const double a = r != 0 ? 1.0 / (r * r) : 1;
        // apply brush - new terrain = weighted average of desired level and previous level,
        // with weight starting at 1 and decreasing with distance squared
        groundMap.at(p) = bed * a + groundMap.at(p) * (1 - a);
    });
}
