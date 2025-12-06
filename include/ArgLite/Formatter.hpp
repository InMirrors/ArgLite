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

#ifndef ARGLITE_ENABLE_FORMATTER
#define ARGLITE_ENABLE_FORMATTER
#endif

namespace ArgLite {

class Formatter {
public:
    static std::string red(std::string_view str, const std::ostream &os = std::cerr) {
        return format(str, ANSI_RED, os);
    }

    static std::string yellow(std::string_view str, const std::ostream &os = std::cerr) {
        return format(str, ANSI_YELLOW, os);
    }

    static std::string bold(std::string_view str, const std::ostream &os = std::cout) {
        return format(str, ANSI_BOLD, os);
    }

    static std::string boldUnderline(std::string_view str, const std::ostream &os = std::cout) {
        return format(str, ANSI_BOLD_UNDERLINE, os);
    }

private:
    static constexpr std::string_view ANSI_RESET          = "\x1b[0m";
    static constexpr std::string_view ANSI_RED            = "\x1b[91m";
    static constexpr std::string_view ANSI_YELLOW         = "\x1b[33m";
    static constexpr std::string_view ANSI_BOLD           = "\x1b[1m";
    static constexpr std::string_view ANSI_BOLD_UNDERLINE = "\x1b[1m\x1b[4m";

    static std::string format(std::string_view str, std::string_view code, const std::ostream &os = std::cout) {
        std::string result;
        result.reserve(str.size());
        if (shouldFormat(os)) {
            result.append(code).append(str).append(ANSI_RESET);
        } else {
            result.append(str);
        }

        return result;
    }

    static bool shouldFormat(const std::ostream &os) {
        auto *buf = os.rdbuf();
        if (buf == std::cout.rdbuf()) return static_cast<bool>(ISATTY(FILENO(stdout)));
        if (buf == std::cerr.rdbuf()) return static_cast<bool>(ISATTY(FILENO(stderr)));
        return false; // default
    }
};

}; // namespace ArgLite
