#ifndef SDLPP_H_
#define SDLPP_H_

#include "Color.h"
#include "SkyError.h"

#include <mist/Point.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <memory>
#include <vector>

namespace sky
{

class Scene;
class Object;
class Drawable;

class Renderer
{
public:
    Renderer() = default;
    Renderer(SDL_Renderer *);
    ~Renderer();
    Renderer(const Renderer &other) = delete;
    Renderer &operator=(const Renderer &other) = delete;
    Renderer(Renderer &&other) = default;
    Renderer &operator=(Renderer &&other) = default;

    operator SDL_Renderer *() { return renderer; }

    void setDrawColor(const Color &color);
    void clear();
    void present();

private:
    friend class Sky;
    SDL_Renderer *renderer = nullptr;
};

struct RendererDeleter {
    void operator()(SDL_Renderer *r)
    {
        if (r) SDL_DestroyRenderer(r);
    }
};

// -----------------------------------------------------------------------------

class Sky
{
public:
    static void initWindow(const char *title, int width, int height);

    static Sky          &getInstance();
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
    Scene                      *activeScene = nullptr;

    int width;
    int height;

    SDL_Window  *window = nullptr;
    Renderer     renderer;
    SDL_Surface *primarySurface = nullptr;

    void createWindow(const char *name);
};

// -----------------------------------------------------------------------------

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

    [[nodiscard]] SDL_Surface *get() const noexcept { return surface; }

    [[nodiscard]] int getWidth() const noexcept { return surface->w; }
    [[nodiscard]] int getHeight() const noexcept { return surface->h; }

private:
    SDL_Surface *surface;
};

// -----------------------------------------------------------------------------

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

    [[nodiscard]] SDL_Texture *get() const { return texture; }

    [[nodiscard]] int getWidth() const noexcept { return width; }
    [[nodiscard]] int getHeight() const noexcept { return height; }

    void renderTo(Renderer &renderer, const SDL_Rect *src, const SDL_Rect *dest, double angle = 0,
                  const SDL_RendererFlip flip = SDL_FLIP_NONE) const;

private:
    SDL_Texture *texture {nullptr};
    int          width {0};
    int          height {0};
};

// -----------------------------------------------------------------------------

class Font
{
public:
    Font(TTF_Font *font);
    ~Font();

    Font(const Font &other) = delete;
    Font &operator=(const Font &other) = delete;
    Font(Font &&other) = default;
    Font &operator=(Font &&other) = default;

    Texture       renderSolid(const std::string &text, const Color &color) const;
    mist::Point2i measure(const std::string text) const;

private:
    TTF_Font *font;
};

// -----------------------------------------------------------------------------

class Scene
{
public:
    Scene() = default;
    virtual ~Scene() = default;
    Scene(const Scene &) = default;
    Scene(Scene &&) = default;
    Scene &operator=(const Scene &) = default;
    Scene &operator=(Scene &&) = default;

    [[nodiscard]] bool isAlive() const { return alive; }

    void load() { onLoad(); }
    void kill() { alive = false; }
    void draw(Renderer &renderer) { onDraw(renderer); }
    void update(float dt) { onUpdate(dt); }
    void processEvents();

protected:
    virtual void onLoad() {}
    virtual void onDraw(Renderer &) {}
    virtual void onUpdate(float) {}
    virtual void onKeyDown(const SDL_KeyboardEvent &) {}

private:
    bool alive {true};
};

class Assets
{
public:
    auto loadTexture(const char *file) -> std::shared_ptr<Texture>;
    auto loadFont(const char *file, int size) -> std::shared_ptr<Font>;
};

}; // namespace sky

#endif
