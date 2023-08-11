#ifndef SKY_UI_H_
#define SKY_UI_H_

#include "Color.h"
#include "Sky.h"
#include <mist/Point.h>

#include <SDL2/SDL_ttf.h>
#include <memory>
#include <string>
#include <vector>

namespace sky
{

class Font
{
public:
    Font(const char *file, int size);
    ~Font();

    Font(const Font &other) = delete;
    Font &operator=(const Font &other) = delete;
    Font(Font &&other);
    Font &operator=(Font &&other);

    Texture renderSolid(const std::string text, SDL_Color color) const;
    mist::Point2i measure(const std::string text) const;

private:
    TTF_Font *font;
};

/* -------------------------------------------------------------------------- */

class Ui;

class UiDrawable
{
public:
    virtual ~UiDrawable() = default;
    virtual void draw(Ui &context, SDL_Renderer *renderer, int x, int y, double angle) = 0;
    virtual void measure(const Ui &context) = 0;

    int width {0};
    int height {0};
};

/* -------------------------------------------------------------------------- */

/*
template <typename W, typename B>
std::shared_ptr<W> build(B block)
{
    auto ptr = std::make_shared<W>();
    block(*ptr.get());
    return ptr;
}
*/

class Widget : public UiDrawable
{
public:
    void draw(Ui &context, SDL_Renderer *renderer, int x, int y, double angle) override;

    // TODO add getters
    void setFont(const std::shared_ptr<Font> font_) noexcept
    {
        font = font_;
    }

    void setForegroundColor(SDL_Color foregroundColor_) noexcept
    {
        foregroundColor = foregroundColor_;
    }

    void setBackgroundColor(SDL_Color backgroundColor_) noexcept
    {
        backgroundColor = backgroundColor_;
    }

    void invalidate()
    {
        valid = false;
    }

protected:
    std::shared_ptr<Font> font;
    SDL_Color foregroundColor {255, 255, 255, 255};
    SDL_Color backgroundColor {0, 0, 0, 255};

    Texture texture;

    // TODO locking
    virtual Texture render(Ui &context) = 0;

private:
    bool valid {false};
};

/* -------------------------------------------------------------------------- */

struct WidgetSpec {
    int width {0};
    int height {0};
};

struct LabelSpec : public WidgetSpec {
    struct WidgetSpec widget;
    std::string text;
};

class Label : public Widget
{
public:
    Label(const std::string text = "");
    Label(const char *text);

    template <class S>
    Label(const S &spec)
    {
        if constexpr (requires { spec.text; }) text = spec.text;
        if constexpr (requires { spec.width; }) width = spec.width;
        if constexpr (requires { spec.height; }) height = spec.height;
    }

    void measure(const Ui &context) override;

    void setText(const std::string &text_) noexcept
    {
        text = text_;
        invalidate();
    }

private:
    std::string text;

    Texture render(Ui &context) override;
};

/* -------------------------------------------------------------------------- */

class LinearLayout : public UiDrawable
{
public:
    LinearLayout() = default;
    LinearLayout(int width, int height) noexcept;

    LinearLayout &operator+=(UiDrawable *d);
    void draw(Ui &context, SDL_Renderer *renderer, int x, int y, double angle) override;
    void measure(const Ui &context) override;

private:
    bool vertical {true};
    std::vector<UiDrawable *> contents;
};

/* -------------------------------------------------------------------------- */

class Ui : public Drawable
{
public:
    Ui(std::shared_ptr<Font> defaultFont);
    void draw(SDL_Renderer *renderer, int x, int y, double angle) override;
    void measure();

    [[nodiscard]] auto getDefaultFont() const noexcept
    {
        return defaultFont;
    }

    void setRoot(std::shared_ptr<UiDrawable> root_) noexcept
    {
        root = root_;
    }

private:
    std::shared_ptr<Font> defaultFont;
    std::shared_ptr<UiDrawable> root;
};

} // namespace sky

#endif
