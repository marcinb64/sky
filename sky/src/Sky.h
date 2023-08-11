#ifndef SDLPP_H_
#define SDLPP_H_

#include <SDL2/SDL.h>
#include <exception>
#include <memory>
#include <string>
#include <vector>

namespace sky
{

class Scene;
class Object;
class Drawable;

class Sky
{
public:
    static void initWindow(const char *title, int width, int height);

    static Sky &getInstance();
    static SDL_Renderer *currentRenderer();

    Sky();
    ~Sky();
    Sky(const Sky &) = delete;
    Sky(Sky &&) = delete;
    Sky &operator=(const Sky &) = delete;
    Sky &operator=(Sky &&) = delete;

    void mainLoop(float fpsCap);

    void setScene(Scene *scene);

private:
    static std::unique_ptr<Sky> instance;
    Scene *activeScene = nullptr;

    int width;
    int height;

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Surface *primarySurface = nullptr;

    void createWindow(const char *name);
};

/* -------------------------------------------------------------------------- */

class Surface
{
public:
    Surface(SDL_Surface *surface);
    Surface(int width, int height, int bpp);
    ~Surface();

    Surface(const Surface &other) = delete;
    Surface &operator=(const Surface &other) = delete;
    Surface(Surface &&other) noexcept;
    Surface &operator=(Surface &&other) noexcept;

    [[nodiscard]] SDL_Surface *get() const noexcept
    {
        return surface;
    }

    [[nodiscard]] int getWidth() const noexcept
    {
        return surface->w;
    }
    [[nodiscard]] int getHeight() const noexcept
    {
        return surface->h;
    }

private:
    SDL_Surface *surface;
};

/* -------------------------------------------------------------------------- */

class Texture
{
public:
    Texture() = default;
    Texture(SDL_Texture *texture, int width, int height);
    Texture(const Surface &surface);
    ~Texture();

    Texture(const Texture &other) = delete;
    Texture &operator=(const Texture &other) = delete;
    Texture(Texture &&other) noexcept;
    Texture &operator=(Texture &&other) noexcept;

    [[nodiscard]] SDL_Texture *get() const
    {
        return texture;
    }

    [[nodiscard]] int getWidth() const noexcept
    {
        return width;
    }
    [[nodiscard]] int getHeight() const noexcept
    {
        return height;
    }

    void renderTo(SDL_Renderer *renderer, const SDL_Rect *src, const SDL_Rect *dest,
                  const SDL_RendererFlip flip = SDL_FLIP_NONE) const;

private:
    SDL_Texture *texture {nullptr};
    int width {0};
    int height {0};
};

/* -------------------------------------------------------------------------- */

struct RendererDeleter {
    void operator()(SDL_Renderer *r)
    {
        if (r) SDL_DestroyRenderer(r);
    }
};

/* -------------------------------------------------------------------------- */

class Scene
{
public:
    Scene() = default;
    virtual ~Scene() = default;
    Scene(const Scene &) = default;
    Scene(Scene &&) = default;
    Scene &operator=(const Scene &) = default;
    Scene &operator=(Scene &&) = default;

    [[nodiscard]] bool isAlive() const
    {
        return alive;
    }

    virtual void onLoad() {}

    void kill()
    {
        alive = false;
    }

    void add(std::shared_ptr<Object> d);
    void draw(SDL_Renderer *renderer);
    void update(float dt);
    void processEvents();

protected:
    virtual void onKeyDown(const SDL_KeyboardEvent &) {}
    virtual void onUpdate(float) {}

private:
    bool alive {true};

    std::vector<std::shared_ptr<Object>> renderList;
};

/* -------------------------------------------------------------------------- */

class Drawable
{
public:
    Drawable() = default;
    Drawable(const Drawable &) = default;
    Drawable(Drawable &&) = default;
    Drawable &operator=(const Drawable &) = default;
    Drawable &operator=(Drawable &&) = default;
    virtual ~Drawable() = default;

    virtual void draw(SDL_Renderer *renderer, int x, int y, double angle) = 0;
};

/* -------------------------------------------------------------------------- */

class Object
{
public:
    static std::shared_ptr<Object> from(std::shared_ptr<Drawable> d);

    Object() = default;
    explicit Object(std::shared_ptr<Drawable> d);

    Object &setDrawable(std::shared_ptr<Drawable> d);
    Object &setPosition(int x, int y);
    Object &setHeading(double a);

    void draw(SDL_Renderer *renderer) const;

private:
    int x = 0;
    int y = 0;
    double heading = 0;
    std::shared_ptr<Drawable> drawable;
};

/* -------------------------------------------------------------------------- */

class Sprite : public Drawable
{
public:
    Sprite(const char *file);
    ~Sprite() override;
    Sprite(const Sprite &) = delete;
    Sprite(Sprite &&) = default;
    Sprite &operator=(const Sprite &) = delete;
    Sprite &operator=(Sprite &&) = default;

    void draw(SDL_Renderer *renderer, int x, int y, double angle) override;

private:
    SDL_Texture *texture;
    int width;
    int height;
};

/* -------------------------------------------------------------------------- */

class Assets
{
public:
    auto loadSprite(const char *file)
    {
        return std::make_shared<Sprite>(file);
    }
};

/* -------------------------------------------------------------------------- */

class SDLError : public std::exception
{
public:
    SDLError();
    explicit SDLError(const char *m);

    [[nodiscard]] const char *what() const noexcept override
    {
        return message.c_str();
    }

private:
    std::string message;
};

/* -------------------------------------------------------------------------- */

class TTFError : public std::exception
{
public:
    TTFError();
    explicit TTFError(const char *m);

    [[nodiscard]] const char *what() const noexcept override
    {
        return message.c_str();
    }

private:
    std::string message;
};

/* -------------------------------------------------------------------------- */

class IMGError : public std::exception
{
public:
    IMGError();
    explicit IMGError(const char *m);

    [[nodiscard]] const char *what() const noexcept override
    {
        return message.c_str();
    }

private:
    std::string message;
};

/* -------------------------------------------------------------------------- */

}; // namespace sky

#endif
