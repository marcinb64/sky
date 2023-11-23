#include <SkyEngine.h>
#include <SkyUi.h>
#include <Tiles.h>
#include <mist/Matrix.h>
#include <mist/Noise.h>
#include <spdlog/spdlog.h>

namespace demo
{

static constexpr auto FPS_CAP = 30.0;

static constexpr auto tileSize = 32;
static constexpr auto numDirtTiles = 8;
static constexpr auto numGrassTiles = 16;

struct TilesetLoader {
    using Arg = nullptr_t;

    sky::SharedTileset operator()()
    {
        using namespace sky::TilePainters;
        sky::TilesetBuilder builder {tileSize};

        auto dirtGradient = hsvValueRamp(50.0f, 0.6f, 0.2f, 0.5f, numDirtTiles);
        auto grassGradient = hsvValueRamp(120.0f, 0.5f, 0.3f, 1.0f, numGrassTiles);
        builder.addSequence(numDirtTiles, dirtGradient);
        builder.addSequence(numGrassTiles, grassGradient);

        return builder.build();
    }
};

struct DemoAssets {
    static inline const sky::SpriteRes selector {"res/selection.png"};
    static inline const sky::FontRes   primaryFont {sky::FontSpec {"res/default.ttf", 14}};
    static inline const sky::Res<sky::Tileset, TilesetLoader> tileset;
};

class TilesDemoScene : public sky::EngineScene
{
public:
    TilesDemoScene() {}

    void onLoad() override
    {
        auto tileset = DemoAssets::tileset.get();
        tilemap = std::make_shared<sky::TileMap>(tileset, 32, 32);
        add(sky::Object::from(tilemap));

        selector = sky::Object::from(DemoAssets::selector.get());
        selector->position = {0.5, 0.5};
        add(selector);

        generateMap();
    }

    void generateMap()
    {
        mist::Matrix<double> noiseMap(32, 32);
        mist::PerlinNoise2   perlin;
        perlin.setSeed(std::random_device {}());

        mist::NoiseTextureBuilder2(noiseMap, perlin)
            .setNoiseScale(1.5)
            .setXScale(4.0)
            .setYScale(4.0)
            .build();

        auto tilePainter = [&](const mist::Point2i &p, double v) {
            tilemap->at(p) =
                mist::roundi(std::clamp(v, 0.0, 1.0) * (numDirtTiles + numGrassTiles - 1));
        };

        noiseMap.foreachKeyValue(tilePainter);
    }

    void onKeyDown(const SDL_KeyboardEvent &event) override
    {
        switch (event.keysym.scancode) {
        case SDL_SCANCODE_ESCAPE: kill(); break;
        case SDL_SCANCODE_UP: selector->position.y--; break;
        case SDL_SCANCODE_DOWN: selector->position.y++; break;
        case SDL_SCANCODE_LEFT: selector->position.x--; break;
        case SDL_SCANCODE_RIGHT: selector->position.x++; break;
        default: break;
        }
    }

private:
    sky::SharedTileMap tilemap;
    sky::SharedObject  selector;
};

} // namespace demo

int main()
{
    spdlog::set_level(spdlog::level::debug);

    try {
        using sky::Sky;
        using namespace demo;

        Sky::initWindow("Sky - Tiles demo", 1200, 700);
        TilesDemoScene scene {};
        scene.setWorldToScreen(sky::Transforms::tiles(tileSize, 1200, 700));
        Sky::getInstance().setScene(&scene);
        Sky::getInstance().mainLoop(FPS_CAP);
    }

    catch (std::exception &e) {
        spdlog::critical(e.what());
    }

    return 0;
}
