#ifndef SKY_UI_H_
#define SKY_UI_H_

#include "Color.h"
#include "Sky.h"
#include "SkyEngine.h"
#include <mist/Point.h>

#include <SDL2/SDL_ttf.h>
#include <memory>
#include <string>
#include <vector>

namespace sky
{

class Ui;

class UiDrawable
{
public:
    virtual ~UiDrawable() = default;
    virtual void draw(Ui &context, Renderer &renderer, int x, int y, double angle) = 0;
    virtual void measure(const Ui &context) = 0;

    int width {0};
    int height {0};
};

/* -------------------------------------------------------------------------- */

class Widget : public UiDrawable
{
public:
    void draw(Ui &context, Renderer &renderer, int x, int y, double angle) override;

    void               setFont(const std::shared_ptr<Font> font_) noexcept { font = font_; }
    [[nodiscard]] auto getFont() const noexcept -> std::shared_ptr<Font> { return font; }

    void               setForegroundColor(Color color) noexcept { foregroundColor = color; }
    [[nodiscard]] auto getForegroundColor() const noexcept -> Color { return foregroundColor; }

    void               setBackgroundColor(Color color) noexcept { backgroundColor = color; }
    [[nodiscard]] auto getBackgroundColor() const noexcept -> Color { return backgroundColor; }

    void invalidate() { valid = false; }

protected:
    std::shared_ptr<Font> font;
    Color                 foregroundColor {255, 255, 255, 255};
    Color                 backgroundColor {0, 0, 0, 255};

    Texture texture;

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
    std::string text;
};

class Label : public Widget
{
public:
    Label(const std::string text = "");
    Label(const char *text);

    template <class S> Label(const S &spec)
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
    void          draw(Ui &context, Renderer &renderer, int x, int y, double angle) override;
    void          measure(const Ui &context) override;

private:
    bool                      vertical {true};
    std::vector<UiDrawable *> contents;
};

/* -------------------------------------------------------------------------- */

class Ui : public Drawable
{
public:
    Ui(std::shared_ptr<Font> defaultFont);
    void draw(Renderer &renderer, int x, int y, double angle) override;
    void measure();

    [[nodiscard]] auto getDefaultFont() const noexcept { return defaultFont; }

    void setRoot(std::shared_ptr<UiDrawable> root_) noexcept { root = root_; }

private:
    std::shared_ptr<Font>       defaultFont;
    std::shared_ptr<UiDrawable> root;
};

} // namespace sky

#endif
