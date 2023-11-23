#include <Sky.h>
#include <spdlog/spdlog.h>

namespace demo
{

constexpr auto FPS_CAP = 60.0;

class DemoAssets
{
public:
    static const inline sky::TextureRes playerTex {"res/target.png"};
    static const inline sky::FontRes    primaryFont {sky::FontSpec {"res/default.ttf", 14}};
};

class StarterScene : public sky::Scene
{
public:
    StarterScene()
    {
        position.x = 200;
        position.y = 600;
    }

    void onDraw(sky::Renderer &renderer)
    {
        renderer.setDrawColor(sky::Color {0, 0, 0, 255});
        renderer.clear();
        drawStuff(renderer);
        renderer.present();
    }

    void drawStuff(sky::Renderer &renderer)
    {
        const auto &p = DemoAssets::playerTex.get();
        auto        intPos = mist::round(position);
        SDL_Rect    destRect {intPos.x, intPos.y, p->getWidth(), p->getHeight()};
        p->renderTo(renderer, nullptr, &destRect);

        auto labelTex = DemoAssets::primaryFont.get()->renderSolid(
            fmt::format("Pos: {}, {}", intPos.x, intPos.y), sky::Color {255, 255, 0, 255});
        destRect = {10, 20, labelTex.getWidth(), labelTex.getHeight()};

        labelTex.renderTo(renderer, nullptr, &destRect);
    }

    void onUpdate(float dt) override
    {
        position.x = std::clamp(position.x + velocity.x * dt, 0.0f, 1200.0f);
        position.y = std::clamp(position.y + velocity.y * dt, 0.0f, 800.0f);
    }

    void onKeyDown(const SDL_KeyboardEvent &event) override
    {
        switch (event.keysym.scancode) {
        case SDL_SCANCODE_ESCAPE: kill(); break;
        case SDL_SCANCODE_LEFT: velocity.x = -100.0f; break;
        case SDL_SCANCODE_RIGHT: velocity.x = 100.0f; break;
        case SDL_SCANCODE_DOWN: velocity.x = 0.0f; break;
        default: break;
        }
    }

private:
    mist::Point2f position;
    mist::Point2f velocity;
};

} // namespace demo

int main()
{
    spdlog::set_level(spdlog::level::debug);

    try {
        using sky::Sky;
        using namespace demo;

        Sky::initWindow("Sky - Basic demo", 1200, 800);
        StarterScene scene {};
        Sky::getInstance().setScene(&scene);
        Sky::getInstance().mainLoop(FPS_CAP);
    }

    catch (std::exception &e) {
        spdlog::critical(e.what());
    }

    return 0;
}
