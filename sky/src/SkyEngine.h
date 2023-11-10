#ifndef SKYENGINE_H_
#define SKYENGINE_H_

#include "Sky.h"

#include <memory>
#include <vector>

struct SDL_Renderer;

namespace sky
{

class Object;

class EngineScene : public Scene
{
public:
    void add(std::shared_ptr<Object> d);

protected:
    void onDraw(Renderer &renderer) override;

private:
    std::vector<std::shared_ptr<Object>> renderList;
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
    static std::shared_ptr<Object> from(std::shared_ptr<Drawable> d);

    Object() = default;
    explicit Object(std::shared_ptr<Drawable> d);

    Object &setDrawable(std::shared_ptr<Drawable> d);
    Object &setPosition(int x, int y);
    Object &setHeading(double a);

    [[nodiscard]] auto getX() const noexcept -> int { return x; }
    [[nodiscard]] auto getY() const noexcept -> int { return y; }

    void draw(Renderer &renderer) const;

private:
    int                       x = 0;
    int                       y = 0;
    double                    heading = 0;
    std::shared_ptr<Drawable> drawable;
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


} // namespace sky

#endif
