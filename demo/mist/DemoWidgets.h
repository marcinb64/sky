#ifndef DEMOWIDGETS_H_
#define DEMOWIDGETS_H_

#include <mist/Value.h>
#include <Sky.h>
#include <SkyUi.h>
#include <Tiles.h>

#include <spdlog/spdlog.h>

namespace demo
{

auto renderLongNum(const long &v)
{
    return fmt::format("{}", v);
}

auto renderDoubleNum(const double &v)
{
    return fmt::format("{:.3f}", v);
}

struct Tweakable {
    virtual ~Tweakable() = default;
    virtual auto increase() -> void = 0;
    virtual auto decrease() -> void = 0;
};

struct TweakableWidget : public sky::Label {
    virtual ~TweakableWidget() = default;
    virtual auto increase() -> void = 0;
    virtual auto decrease() -> void = 0;
};

template <typename T> class ParameterWidget : public TweakableWidget, public mist::Owner
{
public:
    using Renderer = std::function<std::string(const T &)>;

    ParameterWidget(const std::string &label_, mist::Value<T> &value, Renderer renderer_)
        : label(label_), renderer(renderer_)
    {
        value.watch(this, [&](const T &v) {
            show(v);
        });
    }

    auto show(const T &value) { setText(label + renderer(value)); }

    auto attach(std::unique_ptr<Tweakable> controller_) { controller = std::move(controller_); }
    auto increase() -> void override { controller->increase(); }
    auto decrease() -> void override { controller->decrease(); }

private:
    std::string                label;
    Renderer                   renderer;
    std::unique_ptr<Tweakable> controller = nullptr;
};

template <typename T> class ParameterController : public Tweakable
{
public:
    ParameterController(mist::Value<T> &value_, T increment_, T minValue_, T maxValue_)
        : value(value_), increment(increment_), minValue(minValue_), maxValue(maxValue_)
    {
    }

    auto attachDisplay(ParameterWidget<T> *widget_) { widget = widget_; }

    auto increase() -> void override { value = std::min(maxValue, value + increment); }

    auto decrease() -> void override { value = std::max(minValue, value - increment); }

private:
    mist::Value<T> &value;
    T               increment;
    T               minValue;
    T               maxValue;

    ParameterWidget<T> *widget = nullptr;
};

/* -------------------------------------------------------------------------- */

class MapViewer : public sky::Object
{
public:
    MapViewer(const sky::Tileset &tileset, std::shared_ptr<sky::TileMap> tilemap_)
        : tilemap(tilemap_), tileSize(tileset.getTileSize())
    {
        setDrawable(tilemap);
    }

    [[nodiscard]] int getTileSize() const noexcept { return tileSize; }

    [[nodiscard]] int getWidth() const noexcept { return tilemap->getWidth(); }
    [[nodiscard]] int getHeight() const noexcept { return tilemap->getHeight(); }

private:
    std::shared_ptr<sky::TileMap> tilemap;
    int                           tileSize;
};

}

#endif
