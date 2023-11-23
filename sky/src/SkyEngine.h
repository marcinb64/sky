#ifndef SKYENGINE_H_
#define SKYENGINE_H_

#include "Sky.h"

#include <mist/Point.h>
#include <mist/moremath.h>

#include <array>
#include <memory>
#include <vector>

struct SDL_Renderer;

namespace sky
{

class Object;
class Drawable;

using SharedObject = std::shared_ptr<Object>;
using SharedDrawable = std::shared_ptr<Drawable>;

using Transform2d = mist::LinearTransform2<double>;

class Context
{
public:
    Context() = default;

    void          setWorldToScreen(Transform2d w2s) { worldToScreenTransform = w2s; }
    mist::Point2i worldToScreen(mist::Point2d p);

private:
    Transform2d worldToScreenTransform {{0, 0}, {1, 1}, {0, 0}, {1, 1}};
};

class Drawable
{
public:
    Drawable() = default;
    Drawable(const Drawable &) = default;
    Drawable(Drawable &&) = default;
    Drawable &operator=(const Drawable &) = default;
    Drawable &operator=(Drawable &&) = default;
    virtual ~Drawable() = default;

    virtual void draw(Renderer &renderer, int x, int y, double angle) = 0;
};

class Object
{
public:
    mist::Point2d position;
    double        heading = 0;

    static SharedObject from(SharedDrawable d);

    Object() = default;
    explicit Object(SharedDrawable d);

    Object &setDrawable(SharedDrawable d);

    void draw(Renderer &renderer, Context &context) const;

private:
    SharedDrawable drawable;
};

class Sprite : public Drawable
{
public:
    Sprite(std::shared_ptr<Texture> texture);
    ~Sprite() = default;
    Sprite(const Sprite &) = default;
    Sprite(Sprite &&) = default;
    Sprite &operator=(const Sprite &) = default;
    Sprite &operator=(Sprite &&) = default;

    void draw(Renderer &renderer, int x, int y, double angle) override;

private:
    std::shared_ptr<Texture> texture;
    int                      width;
    int                      height;
};

using SharedSprite = std::shared_ptr<Sprite>;

// -----------------------------------------------------------------------------

struct RenderLayer {
    Context                   context;
    std::vector<SharedObject> objects;
};

struct Transforms
{
    static Transform2d uiDefault();
    static Transform2d world(double xExtent, double yExtent, int screenWidth, int screenHeight);
    static Transform2d tiles(int tileSize, int screenWidth, int screenHeight);
};

class EngineScene : public Scene
{
public:
    EngineScene();

    void setWorldToScreen(const Transform2d &w2s);
    void add(SharedObject d);
    void addUi(SharedObject d);

    static SharedSprite loadSprite(const char *file);

protected:
    void         onDraw(Renderer &renderer) override;
    virtual void onPostDraw(Renderer &) {};

private:
    std::array<RenderLayer, 2> layers;
};

struct SpriteLoader {
    using Arg = const char*;

    std::shared_ptr<sky::Sprite> operator()(const char *source)
    {
        return EngineScene::loadSprite(source);
    }
};

using SpriteRes = Res<Sprite, SpriteLoader>;

} // namespace sky

#endif
