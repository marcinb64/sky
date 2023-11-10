#ifndef SKYERROR_H_
#define SKYERROR_H_

#include <exception>
#include <string>

namespace sky
{

class SDLError : public std::exception
{
public:
    SDLError();
    explicit SDLError(const char *m);

    [[nodiscard]] const char *what() const noexcept override { return message.c_str(); }

private:
    std::string message;
};

// -----------------------------------------------------------------------------

class TTFError : public std::exception
{
public:
    TTFError();
    explicit TTFError(const char *m);

    [[nodiscard]] const char *what() const noexcept override { return message.c_str(); }

private:
    std::string message;
};

// -----------------------------------------------------------------------------

class IMGError : public std::exception
{
public:
    IMGError();
    explicit IMGError(const char *m);

    [[nodiscard]] const char *what() const noexcept override { return message.c_str(); }

private:
    std::string message;
};


}

#endif
