#ifndef TERRAFORM_H_
#define TERRAFORM_H_

#include <mist/Matrix.h>
#include <mist/Point.h>
#include <mist/moremath.h>
#include <list>

namespace demo
{

struct NoiseSettings {
    int    numOctaves {2};
    double xyScale {1.0};
    double noiseScale {1.0};
    double roughness {0.75};
    double frequencyMultiplier {2};
};

struct WaterSettings {
    int    biome {-1};
    double routeCost {6.0};
    double riverMaxSize {10.0};
    double riverDepth {0.25};
};

class TileProvider
{
public:
    virtual ~TileProvider() = default;
    virtual auto getBiomeTile(int biome, double value) -> int = 0;
    virtual auto getTransitionTile(int tileA, int tileB, double progress) -> int = 0;
};

/* -------------------------------------------------------------------------- */

class RiverRouter
{
    using Point2i = mist::Point2i;

public:
    RiverRouter(mist::Matrix<double> &groundMap_) : groundMap(groundMap_) {}

    auto setWaterSettings(const WaterSettings &settings) -> RiverRouter &;
    auto findRiverRoute(float minDistance) -> std::list<Point2i>;
    auto digTrenchAlong(const std::list<mist::Point2i> &route, int radius, double level) -> void;

private:
    mist::Matrix<double> &groundMap;
    WaterSettings         settings;

    auto findEndpointCandidates(const Point2i &p0, const Point2i &step, double maxLevel,
                                int minDistance) -> std::vector<Point2i>;
};

/* -------------------------------------------------------------------------- */

class Terraformer
{
public:
    struct BiomeSelector {
        double                        endValue;
        int                           biome;
        mist::LinearTransform<double> noiseToValue {0, 0, 0, 0};

        BiomeSelector(double start, double end, int biome_)
            : endValue(end), biome(biome_), noiseToValue(start, end, 0, 1)
        {
        }
    };

    Terraformer(const mist::Size &mapSize, TileProvider &tileProvider_);

    [[nodiscard]] auto getTileMap() const noexcept -> const mist::Matrix<int> & { return tileMap; }

    auto setSeed(long seed) -> Terraformer &;
    auto setNoiseSettings(const NoiseSettings &settings) -> Terraformer &;
    auto setWaterSettings(const WaterSettings &settings) -> Terraformer &;
    auto addBiome(int biome, double endValue) -> Terraformer &;
    auto generate() -> void;

private:
    double                     minValue;
    double                     maxValue;
    mist::Matrix<int>          tileMap;
    mist::Matrix<double>       groundMap;
    mist::Matrix<int>          biomeMap;
    TileProvider              &tileProvider;
    std::vector<BiomeSelector> biomes;

    NoiseSettings noiseSettings;
    WaterSettings waterSettings;

    RiverRouter router;
    double      lastBiomeEndValue = -1.0;

    auto makeRiver() -> void;
    auto makeRiverAlong(const std::list<mist::Point2i> &route, double maxDistance,
                        double riverbedLevel, double waterLevel) -> void;
};

/* -------------------------------------------------------------------------- */

} // namespace demo

#endif
