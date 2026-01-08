#pragma once
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

namespace ArgLite {

class Formatter {
public:
    static auto red(std::string_view sv, const std::ostream &os = std::cerr) {
#ifdef ARGLITE_ENABLE_FORMATTER
        return format(sv, ANSI_RED, os);
#else
        return sv;
#endif
    }

    static auto yellow(std::string_view sv, const std::ostream &os = std::cerr) {
#ifdef ARGLITE_ENABLE_FORMATTER
        return format(sv, ANSI_YELLOW, os);
#else
        return sv;
#endif
    }

    static auto bold(std::string_view sv, const std::ostream &os = std::cout) {
#ifdef ARGLITE_ENABLE_FORMATTER
        return format(sv, ANSI_BOLD, os);
#else
        return sv;
#endif
    }

    static auto boldUnderline(std::string_view sv, const std::ostream &os = std::cout) {
#ifdef ARGLITE_ENABLE_FORMATTER
        return format(sv, ANSI_BOLD_UNDERLINE, os);
#else
        return sv;
#endif
    }

#ifdef ARGLITE_ENABLE_FORMATTER
private:
    static constexpr std::string_view ANSI_RESET          = "\x1b[0m";
    static constexpr std::string_view ANSI_RED            = "\x1b[91m";
    static constexpr std::string_view ANSI_YELLOW         = "\x1b[33m";
    static constexpr std::string_view ANSI_BOLD           = "\x1b[1m";
    static constexpr std::string_view ANSI_BOLD_UNDERLINE = "\x1b[1m\x1b[4m";

    static std::string format(std::string_view sv, std::string_view code, const std::ostream &os = std::cout) {
        std::string result;
        result.reserve(sv.size());
        if (shouldFormat(os)) {
            result.append(code).append(sv).append(ANSI_RESET);
        } else {
            result.append(sv);
        }

        return result;
    }

    static bool shouldFormat(const std::ostream &os) {
        auto *buf = os.rdbuf();
        if (buf == std::cout.rdbuf()) return static_cast<bool>(ISATTY(FILENO(stdout)));
        if (buf == std::cerr.rdbuf()) return static_cast<bool>(ISATTY(FILENO(stderr)));
        return false; // default
    }
#endif
};

}; // namespace ArgLite
