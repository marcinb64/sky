#include "MistDemo.h"
#include "DemoWidgets.h"

#include <Noise.h>
#include <Terraform.h>
#include <moremath.h>

#include <Value.h>
#include <sky.h>
#include <skyui.h>
#include <spdlog/spdlog.h>

#include <random>
#include <tiles.h>

using namespace demo;
using Point2f = mist::Point2f;
using Point2i = mist::Point2i;

template <> struct fmt::formatter<mist::Point2i> {
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const mist::Point2i &c, FormatContext &ctx) -> decltype(ctx.out())
    {
        return format_to(ctx.out(), "({}, {})", c.x, c.y);
    }
};

namespace mist
{
auto operator<(const Point2i a, const Point2i b) noexcept -> bool
{
    return a.x < b.x || (a.x <= b.x && a.y < b.y);
}
} // namespace mist

namespace demo
{

/* -------------------------------------------------------------------------- */

constexpr auto FPS_CAP = 60.0;
constexpr auto TILE_SIZE = 4;
constexpr auto MAP_SIZE = 128 + 1;
// constexpr auto TILE_SIZE = 32;
// constexpr auto MAP_SIZE = 8 + 1;
constexpr auto DEFAULT_ROUGHNESS = 0.75;
constexpr auto DEFAULT_INITIAL_RANDOMNESS = 1.0;

/* -------------------------------------------------------------------------- */

#include <Color.h>

class DemoAssets : public sky::Assets
{
    using Font = std::shared_ptr<sky::Font>;

public:
    static constexpr auto waterTilesStart = 100;

    DemoAssets(int tileSize) : tileset(makeTileset(tileSize)) {}

    Font         font {std::make_shared<sky::Font>("res/default.ttf", 14)};
    sky::Tileset tileset;

    sky::Tileset makeTileset(int tileSize)
    {
        static constexpr auto steps = 100;
        using mist::lerp;
        auto grayscale = sky::TilePainters::hsvValueRamp(0.0f, 0.0f, 0.0f, 1.0f, steps);
        auto blueScale = sky::TilePainters::hsvValueRamp(246.0f, 1.0f, 0.0f, 1.0f, steps);

        return sky::TilesetBuilder(tileSize, steps + steps)
            .generateSequence(0, steps, grayscale)
            .generateSequence(steps, steps, blueScale)
            .build();
    }
};

/* -------------------------------------------------------------------------- */

struct UiModel {
    mist::Value<long>   seed;
    mist::Value<double> roughness {DEFAULT_ROUGHNESS};
    mist::Value<double> xyScale {1.5};
    mist::Value<double> noiseScale {1.0};
    mist::Value<double> routeCost {6.0};
    mist::Value<double> riverMaxSize {10.0};
    mist::Value<double> riverDepth {0.25};
};

class InfoUi : public sky::Ui, public mist::Owner
{
public:
    static constexpr SDL_Color DEFAULT_BACKGROUND = {0, 0, 0, 255};
    static constexpr SDL_Color SELECTED_BACKGROUND = {89, 162, 222, 255};
    UiModel                    model;

    InfoUi(const DemoAssets &assets) : sky::Ui(assets.font)
    {
        auto root_ = std::make_shared<sky::LinearLayout>();
        populate(*root_);
        setRoot(root_);
        measure();
        updateSelected();

        seedWidget.attach(
            std::make_unique<ParameterController<long>>(model.seed, 1, 0, 0xfffffffffffffff));
        roughnessWidget.attach(
            std::make_unique<ParameterController<double>>(model.roughness, 0.05, 0.0, 10.0));
        xyScaleWidget.attach(
            std::make_unique<ParameterController<double>>(model.xyScale, 0.1, 0.1, 10.0));
        noiseScaleWidget.attach(
            std::make_unique<ParameterController<double>>(model.noiseScale, 0.1, 0.1, 10.0));
        routeCostWidget.attach(
            std::make_unique<ParameterController<double>>(model.routeCost, 0.5, 1, 100.0));
        riverMaxSizeWidget.attach(
            std::make_unique<ParameterController<double>>(model.riverMaxSize, 1, 1, 100.0));
        riverDepthWidget.attach(
            std::make_unique<ParameterController<double>>(model.riverDepth, 0.05, 0.1, 2.0));
    }

    void populate(sky::LinearLayout &container)
    {
        for (auto &i : settings) {
            i->width = 220;
            container += i;
        }
    }

    void updateSelected()
    {
        for (auto i = 0; static_cast<size_t>(i) < settings.size(); ++i) {
            settings[i]->setBackgroundColor(selected == i ? SELECTED_BACKGROUND
                                                          : DEFAULT_BACKGROUND);
        }
    }

    void nextSetting()
    {
        selected = (selected + 1) % static_cast<int>(settings.size());
        updateSelected();
    }

    void prevSetting()
    {
        if (--selected < 0) selected = static_cast<int>(settings.size()) - 1;
        updateSelected();
    }

    void increaseSetting() { settings.at(selected)->increase(); }

    void decreaseSetting() { settings.at(selected)->decrease(); }

private:
    ParameterWidget<long>   seedWidget {"Seed: ", model.seed, renderLongNum};
    ParameterWidget<double> roughnessWidget {"Roughness: ", model.roughness, renderDoubleNum};
    ParameterWidget<double> xyScaleWidget {"XY Scale: ", model.xyScale, renderDoubleNum};
    ParameterWidget<double> noiseScaleWidget {"Noise Scale: ", model.noiseScale, renderDoubleNum};
    ParameterWidget<double> routeCostWidget {"Route Cost: ", model.routeCost, renderDoubleNum};
    ParameterWidget<double> riverMaxSizeWidget {"River size: ", model.riverMaxSize,
                                                renderDoubleNum};
    ParameterWidget<double> riverDepthWidget {"River depth: ", model.riverDepth, renderDoubleNum};

    std::array<TweakableWidget *, 7> settings {
        &seedWidget,      &roughnessWidget,    &xyScaleWidget,   &noiseScaleWidget,
        &routeCostWidget, &riverMaxSizeWidget, &riverDepthWidget};

    int selected {0};
};

/* -------------------------------------------------------------------------- */

class MistDemoScene : public sky::Scene
{
public:
    MistDemoScene(const DemoAssets &assets_, int mapSize)
        : assets(assets_), groundMap(mapSize, mapSize), waterMap(mapSize, mapSize),
          costMap(mapSize, mapSize)
    {
        tilemap = std::make_shared<sky::TileMap>(assets.tileset, mapSize, mapSize);
        mapViewer = std::make_shared<demo::MapViewer>(assets.tileset, tilemap);
        infoUi = std::make_shared<InfoUi>(assets);

        std::random_device rnd;
        const long         seed = rnd() % 0xfffffffffffffff;
        infoUi->model.seed = seed;

        placeObjects();
        onSettingsChanged();
    }

    void onKeyDown(const SDL_KeyboardEvent &event) override
    {
        switch (event.keysym.scancode) {
        case SDL_SCANCODE_ESCAPE: kill(); break;
        case SDL_SCANCODE_N: generateNoiseMap(); break;
        case SDL_SCANCODE_DOWN: infoUi->nextSetting(); break;
        case SDL_SCANCODE_UP: infoUi->prevSetting(); break;
        case SDL_SCANCODE_EQUALS:
            infoUi->increaseSetting();
            onSettingsChanged();
            break;
        case SDL_SCANCODE_MINUS:
            infoUi->decreaseSetting();
            onSettingsChanged();
            break;
        case SDL_SCANCODE_1:
            algo = 0;
            onSettingsChanged();
            break;
        case SDL_SCANCODE_2:
            algo = 1;
            onSettingsChanged();
            break;
        case SDL_SCANCODE_3:
            algo = 2;
            onSettingsChanged();
            break;
        case SDL_SCANCODE_SPACE:
            river();
            visualizeTerrain();
            break;
        case SDL_SCANCODE_F1:
            viewMode = 0;
            visualizeTerrain();
            break;
        case SDL_SCANCODE_F2:
            viewMode = 1;
            visualizeCost();
            break;
        default: break;
        }
    }

    void onUpdate(float) override {}

    [[nodiscard]] auto getSeed() const noexcept -> long { return infoUi->model.seed; }

private:
    const DemoAssets                &assets;
    std::shared_ptr<demo::MapViewer> mapViewer;
    std::shared_ptr<InfoUi>          infoUi;
    std::shared_ptr<sky::TileMap>    tilemap;

    mist::Matrix<double> groundMap;
    mist::Matrix<double> waterMap;
    mist::Matrix<double> costMap;
    int                  viewMode = 0;
    int                  algo = 1;

    void initUi() {}

    void placeObjects()
    {
        constexpr auto MARGIN = 4;

        auto ui = sky::Object::from(infoUi);
        ui->setPosition(mapViewer->getWidth() * mapViewer->getTileSize() + MARGIN,
                        2 * mapViewer->getTileSize() + MARGIN);
        add(ui);

        add(mapViewer);
    }

    void onSettingsChanged()
    {
        generateNoiseMap();
        if (viewMode == 0) visualizeTerrain();
        if (viewMode == 1) visualizeCost();
    }

    void generateNoiseMap()
    {
        static constexpr std::array algoNames {"Diamond Square", "fBM / Perlin", "Domain Warped"};
        spdlog::info("Generate new map using {} seed {}", algoNames[algo], infoUi->model.seed);

        groundMap.fill(0);
        waterMap.fill(0);

        mist::setPerlinSeed(infoUi->model.seed);

        if (algo == 0) {
            mist::DiamondSquare<double>(groundMap)
                .setSeed(infoUi->model.seed)
                .setRoughness(infoUi->model.roughness)
                .setInitialRandomness(1.0)
                .build();
        } else if (algo == 1) {
            mist::PerlinNoise2 perlin;
            mist::OctaveNoise2 noise(perlin);
            noise.setNumOctaves(6).setRoughness(infoUi->model.roughness);
            mist::NoiseTextureBuilder2(groundMap, noise)
                .setNoiseScale(infoUi->model.noiseScale)
                .setXScale(infoUi->model.xyScale)
                .setYScale(infoUi->model.xyScale)
                .build();
        } else if (algo == 2) {
            mist::PerlinNoise2 perlin;
            mist::OctaveNoise2 octaves(perlin);
            octaves.setNumOctaves(6).setRoughness(infoUi->model.roughness);
            mist::DomainWarpedNoise2 domainWarp(octaves);
            mist::NoiseTextureBuilder2(groundMap, domainWarp)
                .setNoiseScale(infoUi->model.noiseScale)
                .setXScale(infoUi->model.xyScale)
                .setYScale(infoUi->model.xyScale)
                .build();
        }

        river();
    }

    void river()
    {
        const auto riverRoute = findRiverRoute(MAP_SIZE / 2);
        if (!riverRoute.empty()) {
            spdlog::info("Found river route: {} to {}", riverRoute.front(), riverRoute.back());
            const auto bedLevel = groundMap.at(riverRoute.front());
            makeRiverAlong(riverRoute, infoUi->model.riverMaxSize, bedLevel,
                           bedLevel + infoUi->model.riverDepth);
        } else {
            spdlog::info("No suitable river route found");
        }
    }

    auto findRiverRoute(float minDistance) -> std::list<Point2i>
    {
        // Find low points along all edges of the map
        std::vector<Point2i> candidates;

        // Scan for end point candidates along the 4 edges of the map
        // Look for low points that are far enough from each other
        auto tmp = findRiverEndpointCandidates({0, 0}, {0, 1}, -0.25, MAP_SIZE / 8);
        candidates.insert(candidates.end(), tmp.begin(), tmp.end());
        tmp = findRiverEndpointCandidates({MAP_SIZE - 1, 0}, {0, 1}, -0.25, MAP_SIZE / 8);
        candidates.insert(candidates.end(), tmp.begin(), tmp.end());
        tmp = findRiverEndpointCandidates({0, 0}, {1, 0}, -0.25, MAP_SIZE / 8);
        candidates.insert(candidates.end(), tmp.begin(), tmp.end());
        tmp = findRiverEndpointCandidates({0, MAP_SIZE - 1}, {1, 0}, -0.25, MAP_SIZE / 8);
        candidates.insert(candidates.end(), tmp.begin(), tmp.end());

        // Need at least start and end point
        if (candidates.size() < 2) return {};

        for (const auto &start : candidates) {
            // Build a map of the cost to reach each place from the start point
            mist::AStar router(groundMap);
            router
                .setStepCostFactor(infoUi->model.routeCost) // cost of going to higher ground
                .setBlockValue(0.0) // Terrain height is [-1, 1], don't allow river above 0.0
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

                if (!router.canReach(end)) {
                    spdlog::debug("From {} to {} : no route", start, end);
                    continue;
                }

                return router.route(end);
            }
        }

        return {};
    }

    auto findRiverEndpointCandidates(const Point2i &p0, const Point2i &step, double maxLevel,
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

    auto makeRiverAlong(const std::list<mist::Point2i> &route, double maxDistance,
                        double riverbedLevel, double waterLevel) -> void
    {
        waterMap.fill(0);

        digTrenchAlong(route, mist::roundi(maxDistance / 2), riverbedLevel);
        fillRiverAlong(route, maxDistance, waterLevel);
    }

    auto digTrenchAlong(const std::list<mist::Point2i> &route, int radius, double level) -> void
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

    auto fillRiverAlong(const std::list<mist::Point2i> &route, double maxDistance,
                        double waterLevel) -> void
    {
        waterMap.fill(0);

        for (const auto &p0 : route) {
            mist::LinearTransform groundToWater(-1.0, 0.75, 0.35, 1);
            mist::floodFill(groundMap, p0, maxDistance, waterLevel, [&](const Point2i &p) {
                waterMap.at(p) = std::clamp(groundToWater(groundMap.at(p)), 0.0, 1.0);
            });
        }
    }

    void visualizeTerrain()
    {
        mist::LinearTransform toGroundTile(-1.0, 1.0, 0, 99);
        mist::LinearTransform toWaterTile(0.0, 1.0, 100, 199);
        auto                  tilePainter = [&](const mist::Point2i &p, double v) {
            auto w = waterMap.at(p);
            if (w > 0.01) {
                (*tilemap)[p] = mist::roundi(toWaterTile(w));
            } else {
                (*tilemap)[p] = mist::roundi(toGroundTile(v));
            }
        };

        groundMap.foreachKeyValue(tilePainter);
    }

    void visualizeCost()
    {
        const auto minCost = min(costMap);
        const auto maxCost = max(costMap);
        spdlog::debug("Cost range: {} - {}", minCost, maxCost);
        mist::LinearTransform toWaterTile(minCost, maxCost, 100, 199);
        auto                  tilePainter = [&](const mist::Point2i &p, double v) {
            (*tilemap)[p] = mist::roundi(toWaterTile(v));
        };

        costMap.foreachKeyValue(tilePainter);
    }
};

/* -------------------------------------------------------------------------- */

auto mistDemo() -> void
{
    using sky::Sky;

    // mist::Matrix<float> testm(3, 3);
    // testm.at(0, 0) = 3;
    // testm.at(1, 0) = 2;
    // testm.at(2, 0) = 2;

    // testm.at(0, 1) = 2;
    // testm.at(1, 1) = 4;
    // testm.at(2, 1) = 0;

    // testm.at(0, 2) = 1;
    // testm.at(1, 2) = 1;
    // testm.at(2, 2) = 1;
    // mist::AStar router(testm);
    // router.calculate(mist::Point2i { 2, 1 });

    Sky::initWindow("Mist Demo", 1400, 1000);
    DemoAssets    assets(TILE_SIZE);
    MistDemoScene scene {assets, MAP_SIZE};
    Sky::getInstance().setScene(&scene);
    Sky::getInstance().mainLoop(FPS_CAP);

    spdlog::debug("Last seed: {}", scene.getSeed());
}

} // namespace demo