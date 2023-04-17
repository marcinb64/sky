#include "tiledemo.h"
#include "TileEditor.h"

#include <Color.h>
#include <Noise.h>
#include <Matrix.h>
#include <sky.h>
#include <skyui.h>
#include <tiles.h>
#include <moremath.h>

#include <array>
#include <functional>
#include <spdlog/spdlog.h>

using namespace demo;

/* -------------------------------------------------------------------------- */

class DemoAssets : public sky::Assets
{
    using Drw = std::shared_ptr<sky::Drawable>;
    using Font = std::shared_ptr<sky::Font>;

public:
    DemoAssets(int tileSize)
        //: tileset("res/tileset.png", tileSize)
        : tileset(makeTileset(tileSize))
    {
    }

    struct Tiles {
        constexpr static auto WATER_START = 0;
        constexpr static auto WATER_END = 3;
        constexpr static auto SAND = 4;
        constexpr static auto GRASS_START = 5;
        constexpr static auto GRASS_END = 8;
        constexpr static auto DIRT_START = 9;
        constexpr static auto DIRT_END = 12;
    };

    sky::Tileset makeTileset(int tileSize)
    {
        using mist::lerp;
        auto water = sky::TilePainters::plainColor([](int i) {
            return sky::hsv(sky::Hue::Blue, 1.0f, lerp((float)i / 4, 0.3f, 1.0f));
        });
        auto sand = sky::TilePainters::plainColor(sky::hsv(50.0f, 1.0f, 0.8f));
        auto grass = sky::TilePainters::plainColor([](int i) {
            return sky::hsv(sky::Hue::Green, 1.0f, lerp((float)(3 - i) / 4, 0.3f, 1.0f));
        });
        auto dirt = sky::TilePainters::plainColor([](int i) {
            return sky::hsv(50.0f, 0.7f, lerp((float)(3 - i) / 4, 0.2f, 0.7f));
        });

        return sky::TilesetBuilder(tileSize, 16)
            .generateSequence(0, 4, water)
            .generateTile(4, sand)
            .generateSequence(5, 4, grass)
            .generateSequence(9, 4, dirt)
            .build();
    }

    sky::Tileset tileset;
    const int defaultTile = 0;
    // const std::vector<int> tilePalette {40, 41, 42};
    const std::vector<int> tilePalette {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    Drw tileSelectorSprite {loadSprite("res/selection.png")};
    Font font {std::make_shared<sky::Font>("res/default.ttf", 14)};
};

/* -------------------------------------------------------------------------- */

struct Controller {
    std::function<void(double)> changeWaterRandom;
    std::function<void(double)> changeWaterScale;
    std::function<void(double)> changeGroundRandom;
    std::function<void(double)> changeGroundScale;
    std::function<void(double)> changeWaterLevel;
};

class InfoUi : public sky::Ui
{
public:
    static constexpr SDL_Color DEFAULT_BACKGROUND = {0, 0, 0, 255};
    static constexpr SDL_Color SELECTED_BACKGROUND = {89, 162, 222, 255};

    Controller controller;

    InfoUi(const DemoAssets &assets)
        : sky::Ui(assets.font)
    {
        auto root_ = std::make_shared<sky::LinearLayout>();
        populate(*root_);
        setRoot(root_);
        measure();

        updateSelected();
    }

    void populate(sky::LinearLayout &c)
    {
        for (auto &i : all) {
            i->width = 150;
            c += i;
        }
    }

    void showValues(double w, double ws, double g, double gs, double wl)
    {
        waterRandom.setText(fmt::format("Water: {:.2f}", w));
        waterScale.setText(fmt::format("Water scale: {:.2f}", ws));
        groundRandom.setText(fmt::format("Ground: {:.2f}", g));
        groundScale.setText(fmt::format("Ground scale: {:.2f}", gs));
        waterLevel.setText(fmt::format("Water level: {:.2f}", wl));
    }

    void updateSelected()
    {
        for (auto i = 0; static_cast<size_t>(i) < all.size(); ++i) {
            all[i]->setBackgroundColor(selected == i ? SELECTED_BACKGROUND : DEFAULT_BACKGROUND);
        }
    }

    void nextSetting()
    {
        selected = (selected + 1) % static_cast<int>(all.size());
        updateSelected();
    }

    void prevSetting()
    {
        if (--selected < 0) selected = static_cast<int>(all.size()) - 1;
        updateSelected();
    }

    void increaseSetting()
    {
        settingAt(selected)(0.05);
    }

    void decreaseSetting()
    {
        settingAt(selected)(-0.05);
    }

private:
    sky::Label waterRandom;
    sky::Label waterScale;
    sky::Label groundRandom;
    sky::Label groundScale;
    sky::Label waterLevel;
    std::array<sky::Widget *, 5> all {&waterRandom, &waterScale, &groundRandom, &groundScale,
                                      &waterLevel};

    int selected {0};

    std::function<void(double)> &settingAt(int index)
    {
        switch (index) {
        case 0: return controller.changeWaterRandom;
        case 1: return controller.changeWaterScale;
        case 2: return controller.changeGroundRandom;
        case 3: return controller.changeGroundScale;
        case 4: return controller.changeWaterLevel;
        }
        throw 0;
    }
};

/* -------------------------------------------------------------------------- */

class TileEditorScene : public sky::Scene
{
public:
    TileEditorScene(int tileSize, int mapSize)
        : assets(tileSize)
    {
        editor = std::make_shared<demo::TileEditor>(assets.tileset, mapSize, mapSize);
        selector = std::make_shared<demo::TileSelector>(editor, assets.tileSelectorSprite);
        palette = std::make_shared<demo::TilePalette>(assets.tileset);
        infoUi = std::make_shared<InfoUi>(assets);

        initTileset();
        initUi();
        initTerrain();

        placeObjects();
        onSettingsChanged();
    }

    void onKeyDown(const SDL_KeyboardEvent &event) override
    {
        switch (event.keysym.scancode) {
        case SDL_SCANCODE_ESCAPE: kill(); break;
        case SDL_SCANCODE_RIGHT: moveSelector(East); break;
        case SDL_SCANCODE_LEFT: moveSelector(West); break;
        case SDL_SCANCODE_UP: moveSelector(North); break;
        case SDL_SCANCODE_DOWN: moveSelector(South); break;
        case SDL_SCANCODE_X:
            editor->brush(selector->getLocation(), palette->getSelectedTile());
            break;
        case SDL_SCANCODE_PAGEDOWN: palette->selectNext(); break;
        case SDL_SCANCODE_PAGEUP: palette->selectPrevious(); break;
        case SDL_SCANCODE_N: generateNoiseMap(); break;
        case SDL_SCANCODE_RIGHTBRACKET: infoUi->nextSetting(); break;
        case SDL_SCANCODE_LEFTBRACKET: infoUi->prevSetting(); break;
        case SDL_SCANCODE_EQUALS: infoUi->increaseSetting(); break;
        case SDL_SCANCODE_MINUS: infoUi->decreaseSetting(); break;
        default: break;
        }
    }

    void onUpdate(float) override {}

private:
    DemoAssets assets;
    std::shared_ptr<demo::TileEditor> editor;
    std::shared_ptr<demo::TileSelector> selector;
    std::shared_ptr<demo::TilePalette> palette;
    std::shared_ptr<InfoUi> infoUi;

    void initTileset()
    {
        palette->setPalette(assets.tilePalette);
    }

    void initUi()
    {
        infoUi->controller.changeWaterRandom = [&](double d) {
            waterRandom += d;
            onSettingsChanged();
        };

        infoUi->controller.changeWaterScale = [&](double d) {
            waterScale += d;
            onSettingsChanged();
        };

        infoUi->controller.changeGroundRandom = [&](double d) {
            groundRandom += d;
            onSettingsChanged();
        };

        infoUi->controller.changeGroundScale = [&](double d) {
            groundScale += d;
            onSettingsChanged();
        };

        infoUi->controller.changeWaterLevel = [&](double d) {
            waterLevel += d;
            waterLevel = std::clamp(waterLevel, -1.0, 1.0);
            onSettingsChanged();
        };
    }

    void initTerrain()
    {
        editor->fill(assets.defaultTile);
    }

    void placeObjects()
    {
        constexpr auto MARGIN = 4;
        palette->setPosition(editor->getHeight() * editor->getTileSize() + MARGIN, MARGIN);
        add(palette);

        auto ui = sky::Object::from(infoUi);
        ui->setPosition(editor->getWidth() * editor->getTileSize() + MARGIN,
                        2 * editor->getTileSize() + MARGIN);
        add(ui);

        add(editor);
        add(selector);
    }

    void moveSelector(Direction dir)
    {
        selector->move(dir);
        showTileInfo(selector->getLocation());
    }

    void showTileInfo(mist::Point2i) {}

    void onSettingsChanged()
    {
        generateNoiseMap();
        infoUi->showValues(waterRandom, waterScale, groundRandom, groundScale, waterLevel);
    }

    void generateNoiseMap()
    {
        mist::PerlinNoise2 noise;
        mist::Matrix<double> waterMap(editor->getWidth(), editor->getHeight());
        mist::Matrix<double> groundMap(editor->getWidth(), editor->getHeight());

        editor->fill(0);

        // generate water and ground noisemaps
        mist::NoiseTextureBuilder2(waterMap, noise)
            .setNoiseScale(waterScale)
            .build();
        mist::NoiseTextureBuilder2(groundMap, noise)
            .setNoiseScale(groundScale)
            .build();

        // Define mapping from noise values to tiles.
        // The mapping uses -1,1 input range as a baseline. Noise scale and the randomness
        // affect the actual range of input values. An optional normalization step
        // may be added here.
        mist::LinearTransform toWaterTile(-1.0, waterLevel, DemoAssets::Tiles::WATER_START,
                                         DemoAssets::Tiles::SAND);
        mist::LinearTransform toGroundTile(-1.0, 1.0, DemoAssets::Tiles::GRASS_START,
                                          DemoAssets::Tiles::DIRT_END);

        // combine water and ground noisemaps and apply noise to tile transforms
        groundMap.foreachKeyValue([&](const mist::Point2i &p, double v) {
            return waterMap.at(p) < waterLevel ? toWaterTile(waterMap.at(p)) : toGroundTile(v);
        });

        // draw
        groundMap.foreachKeyValue ([&](const mist::Point2i &p, double v) {
            editor->brush(p, mist::roundi(v));
        });
    }

    double waterRandom {2.85};
    double waterScale {1.0};
    double groundRandom {38.45};
    double groundScale {1.0};
    double waterLevel {-0.35};
};

/* -------------------------------------------------------------------------- */

constexpr auto FPS_CAP = 60.0;
constexpr auto TILE_SIZE = 16;
constexpr auto MAP_SIZE = 48;

void demo::tileeditor()
{
    sky::Sky::initWindow("Tile Editor", 1000, 800);
    TileEditorScene scene {TILE_SIZE, MAP_SIZE};
    sky::Sky::getInstance().setScene(&scene);
    sky::Sky::getInstance().mainLoop(FPS_CAP);
}
