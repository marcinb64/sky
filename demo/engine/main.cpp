#include <SkyEngine.h>
#include <SkyUi.h>
#include <spdlog/spdlog.h>

namespace demo
{

constexpr auto FPS_CAP = 60.0;

struct DemoAssets {
    static inline const sky::SpriteRes target {"res/target.png"};
    static inline const sky::FontRes   primaryFont {sky::FontSpec {"res/default.ttf", 14}};
};

class StarterScene : public sky::EngineScene
{
public:
    StarterScene()
    {
        positionLabel = std::make_shared<sky::Label>();

        ui = std::make_shared<sky::Ui>(DemoAssets::primaryFont.get());
        ui->setRoot(positionLabel);
    }

    void onLoad() override
    {
        target = sky::Object::from(DemoAssets::target.get());
        target->position = {0.0, 0.58};
        add(target);

        sky::SharedObject uiObject = sky::Object::from(ui);
        uiObject->position = {8, 8};
        addUi(uiObject);
    }

    void onUpdate(float dt) override
    {
        const auto d = velocity * dt;
        target->position += d;

        target->position.x = std::clamp(target->position.x, -1.0, 1.0);
        target->position.y = std::clamp(target->position.y, -0.58, 0.58);

        positionLabel->setText(spdlog::fmt_lib::format("Position: {:.2f}, {:.2f}",
                                                       target->position.x, target->position.y));
    }

    void onKeyDown(const SDL_KeyboardEvent &event) override
    {
        switch (event.keysym.scancode) {
        case SDL_SCANCODE_ESCAPE: kill(); break;
        case SDL_SCANCODE_LEFT: velocity.x = -0.4f; break;
        case SDL_SCANCODE_RIGHT: velocity.x = 0.4f; break;
        case SDL_SCANCODE_UP: velocity.y = 0.4f; break;
        case SDL_SCANCODE_DOWN: velocity.y = -0.4f; break;
        case SDL_SCANCODE_SPACE: velocity = {0,0}; break;
        default: break;
        }
    }

private:
    sky::SharedObject target;

    std::shared_ptr<sky::Ui>    ui;
    std::shared_ptr<sky::Label> positionLabel;

    mist::Point2d velocity;
};

} // namespace demo

int main()
{
    spdlog::set_level(spdlog::level::debug);

    try {
        using sky::Sky;
        using namespace demo;

        Sky::initWindow("Sky - Engine demo", 1200, 800);
        StarterScene scene {};
        scene.setWorldToScreen(sky::Transforms::world(2.0, 2.0, 1200, 800));
        Sky::getInstance().setScene(&scene);
        Sky::getInstance().mainLoop(FPS_CAP);
    }

    catch (std::exception &e) {
        spdlog::critical(e.what());
    }

    return 0;
}
