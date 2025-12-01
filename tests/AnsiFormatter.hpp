#pragma once
#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#ifdef _WIN32
#include <io.h>
#define ISATTY _isatty // Platform-independent alias
#define FILENO _fileno
#else
#include <unistd.h>
#define ISATTY isatty
#define FILENO fileno
#endif

class AnsiFormatter {
public:
    enum class Mode : uint8_t {
        Auto,
        Always,
        Never
    };

    AnsiFormatter(std::ostream &os = std::cout, Mode mode = Mode::Auto)
        : out_(os), mode_(mode), isTerminal_(terminalChecker(os))
    {
        updateShouldFormat();
    }
    // Delete these explicitly
    AnsiFormatter(const AnsiFormatter &)            = delete;
    AnsiFormatter &operator=(const AnsiFormatter &) = delete;

    enum class Style : uint8_t {
        Reset,         // 0 (SGR parameter, not enum value)
        Bold,          // 1
        Dim,           // 2
        Italic,        // 3
        Underline,     // 4
        Inverse,       // 7, slow blink and rapid blink are skipped
        Conceal,       // 8
        Strikethrough, // 9
        BoldItalic,    // start of combined styles
        BoldUnderline,
        BoldItalicUnderline,
        BoldInverse,
        ItalicUnderline,
    };

    // Foreground colors
    enum class Color : uint8_t {
        Reset,
        Black,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
        BrtBlack,   // bright black
        BrtRed,     // bright red
        BrtGreen,   // bright green
        BrtYellow,  // bright yellow
        BrtBlue,    // bright blue
        BrtMagenta, // bright magenta
        BrtCyan,    // bright cyan
        BrtWhite    // bright white
    };

    // Background colors
    enum class Back : uint8_t {
        Reset,
        Black,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
        BrtBlack,
        BrtRed,
        BrtGreen,
        BrtYellow,
        BrtBlue,
        BrtMagenta,
        BrtCyan,
        BrtWhite
    };

    struct Format {
        Color color = Color::Reset;
        Style style = Style::Reset;
        Back  back  = Back::Reset;

        std::string getAnsiCode() const
        {
            std::string code;
            if (color != Color::Reset) { code += COLOR_CODES_.at(static_cast<int>(color)); }
            if (back != Back::Reset) { code += BACK_CODES_.at(static_cast<int>(back)); }
            if (style != Style::Reset) { code += STYLE_CODES_.at(static_cast<int>(style)); }
            return code;
        }
    };

    void modifyMode(Mode mode)
    {
        mode_ = mode;
        updateShouldFormat();
    }

    // Append a '\n' and the reset sequence automatically.
    void println(std::string_view str, Color color = Color::Reset)
    {
        // print color code only if color provided and shouldFormat_ is true
        if ((color != Color::Reset) && shouldFormat_) {
            out_ << COLOR_CODES_.at(static_cast<int>(color)) << str
                 << COLOR_CODES_.at(static_cast<int>(Color::Reset)) << '\n';
        }
        else {
            out_ << str << '\n';
        }
    }

    // Append a '\n' and the reset sequence automatically.
    void println(std::string_view str, Back color)
    {
        // print color code only if color provided and shouldFormat_ is true
        if ((color != Back::Reset) && shouldFormat_) {
            out_ << BACK_CODES_.at(static_cast<int>(color)) << str
                 << BACK_CODES_.at(static_cast<int>(Back::Reset)) << '\n';
        }
        else {
            out_ << str << '\n';
        }
    }

    // Append a '\n' and the reset sequence automatically.
    void println(std::string_view str, Style style)
    {
        // print style code only if style provided and shouldFormat_ is true
        if ((style != Style::Reset) && shouldFormat_) {
            out_ << STYLE_CODES_.at(static_cast<int>(style)) << str
                 << STYLE_CODES_.at(static_cast<int>(Style::Reset)) << '\n';
        }
        else {
            out_ << str << '\n';
        }
    }

    // Append a '\n' and the reset sequence automatically.
    // Multiple format parameters version.
    void println(std::string_view str, Color color, Style style, Back back = Back::Reset)
    {
        std::string code;
        if (color != Color::Reset) { code += COLOR_CODES_.at(static_cast<int>(color)); }
        if (back != Back::Reset) { code += BACK_CODES_.at(static_cast<int>(back)); }
        if (style != Style::Reset) { code += STYLE_CODES_.at(static_cast<int>(style)); }

        // print ANSI code only if shouldFormat_ is true and code is not empty
        if (shouldFormat_ && !code.empty()) {
            out_ << code
                 << str
                 << COLOR_CODES_.at(static_cast<int>(Color::Reset)) << '\n';
        }
        else {
            out_ << str << '\n';
        }
    }

    // Append a '\n' and the reset sequence automatically.
    // Use the `Format` struct to pass a variable number of format parameters.
    void println(std::string_view str, Format fmt)
    {
        if (shouldFormat_) {
            std::string code = fmt.getAnsiCode();
            if (!code.empty()) {
                out_ << code << str << COLOR_CODES_.at(static_cast<int>(Color::Reset)) << '\n';
            }
            else {
                out_ << str << '\n';
            }
        }
        else {
            out_ << str << '\n';
        }
    }

    // Stream insertion operator for colors
    AnsiFormatter &operator<<(Color color)
    {
        if (shouldFormat_) {
            out_ << COLOR_CODES_.at(static_cast<int>(color));
        }
        return *this;
    }

    // Stream insertion operator for background colors
    AnsiFormatter &operator<<(Back color)
    {
        if (shouldFormat_) {
            out_ << BACK_CODES_.at(static_cast<int>(color));
        }
        return *this;
    }

    // Stream insertion operator for styles
    AnsiFormatter &operator<<(Style style)
    {
        if (shouldFormat_) {
            out_ << STYLE_CODES_.at(static_cast<int>(style));
        }
        return *this;
    }

    // Stream insertion operator for generic types
    template <typename T>
    AnsiFormatter &operator<<(const T &data)
    {
        out_ << data;
        return *this;
    }

    // For manips like endl
    AnsiFormatter &operator<<(std::ostream &(*manip)(std::ostream &))
    {
        out_ << manip;
        return *this;
    }

private:
    std::ostream &out_;
    Mode          mode_;
    bool          isTerminal_;
    bool          shouldFormat_;

    static constexpr std::array<std::string_view, 17> COLOR_CODES_ = {
        "\x1b[0m",
        "\x1b[30m", "\x1b[31m", "\x1b[32m", "\x1b[33m", "\x1b[34m", "\x1b[35m", "\x1b[36m", "\x1b[37m",
        "\x1b[90m", "\x1b[91m", "\x1b[92m", "\x1b[93m", "\x1b[94m", "\x1b[95m", "\x1b[96m", "\x1b[97m"};

    static constexpr std::array<std::string_view, 17> BACK_CODES_ = {
        "\x1b[0m",
        "\x1b[40m", "\x1b[41m", "\x1b[42m", "\x1b[43m", "\x1b[44m", "\x1b[45m", "\x1b[46m", "\x1b[47m",
        "\x1b[100m", "\x1b[101m", "\x1b[102m", "\x1b[103m", "\x1b[104m", "\x1b[105m", "\x1b[106m", "\x1b[107m"};

    static constexpr std::array<std::string_view, 16> STYLE_CODES_ = {
        "\x1b[0m",
        "\x1b[1m", "\x1b[2m", "\x1b[3m", "\x1b[4m", // Bold, Dim, Italic, Underline
        "\x1b[7m", "\x1b[8m", "\x1b[9m",            // Inverse, Conceal, Strikethrough
        "\x1b[1m\x1b[3m", "\x1b[1m\x1b[4m", "\x1b[1m\x1b[3m\x1b[4m", "\x1b[1m\x1b[7m",
        "\x1b[3m\x1b[4m"};

    void updateShouldFormat()
    {
        if (mode_ == Mode::Always) {
            shouldFormat_ = true;
        }
        else if (mode_ == Mode::Auto) {
            shouldFormat_ = isTerminal_;
        }
        else {
            shouldFormat_ = false;
        }
    }

    static bool terminalChecker(const std::ostream &os)
    {
        auto *buf = os.rdbuf();
        if (buf == std::cout.rdbuf()) return static_cast<bool>(ISATTY(FILENO(stdout)));
        if (buf == std::cerr.rdbuf()) return static_cast<bool>(ISATTY(FILENO(stderr)));
        return false; // default
    }
};
