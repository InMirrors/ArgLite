#pragma once

#include "Core.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef ARGLITE_ENABLE_FORMATTER
#include "Formatter.hpp"
#endif

namespace ArgLite {

inline void Parser::preprocess_(int argc, char **argv) { // NOLINT(readability-function-cognitive-complexity)
    argc_ = argc;
    argv_ = argv;

    auto &data = data_;

    std::string_view shortNonFlagOptsStr = mainCmdShortNonFlagOptsStr_;

    // Set up the program name
    if (argc_ > 0) {
        data.cmdName = argv[0];

        // Extract the basename
        if (auto last_slash_pos = data.cmdName.find_last_of("/\\");
            std::string::npos != last_slash_pos) {
            data.cmdName.erase(0, last_slash_pos + 1);
        }
    }

    // Check if there is a subcommand
    int subCmdOffset = 0;
    if (argc > 1) {
        std::string argv1 = argv[1];
        auto        it    = std::find_if(
            subCmdPtrs_.begin(), subCmdPtrs_.end(), [argv1](const SubParser *p) { return p->subCommandName_ == argv1; });
        if (it != subCmdPtrs_.end()) {
            activeSubCmd_ = *it;
            data.cmdName.append(" ").append(argv[1]); // cmdName is now "program subcommand"
            subCmdOffset        = 1;
            shortNonFlagOptsStr = (*it)->subCmdShortNonFlagOptsStr_;
        }
    }

    bool allPositional = false;
    for (int i = 1 + subCmdOffset; i < argc; ++i) {
        std::string arg = argv[i];

        if (allPositional) {
            data.positionalArgsIndices.push_back(i);
            continue;
        }

        if (arg.length() <= 1) { // Not an option, an option has 2 chars at least (e.g., -h)
            data.positionalArgsIndices.push_back(i);
            continue;
        }

        if (arg == "--") {
            allPositional = true;
            continue;
        }

        // Long option
        if (arg.rfind("--", 0) == 0) {
            std::string key = arg;
            std::string value;
            // --opt=val form
            if (auto equalsPos = arg.find('='); equalsPos != std::string::npos) {
                key               = arg.substr(0, equalsPos);
                value             = arg.substr(equalsPos + 1);
                data.options[key] = {i, std::move(value)};
            } else {
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    data.options[key] = {i + 1, ""};
                    i++; // Consume next arg as value
                } else {
                    data.options[key] = {-i, ""}; // Flag
                }
            }
        }
        // Short option(s)
        // Process short options, e.g., -n 123, -ab, -abn 123, -n123, -abn123
        else if (arg.rfind('-', 0) == 0) {
            std::string lastFlagKey;
            bool        isValueConsumedInCurrentArg = false; // True if a short option like -n123 was found

            for (size_t j = 1; j < arg.length(); ++j) {
                std::string currentOptKey = "-";
                currentOptKey += arg[j];

                // Check if the current character is a short option that requires a value
                if (shortNonFlagOptsStr.find(arg[j]) != std::string::npos && j + 1 < arg.length()) {
                    // `-n123` or `-abn123` form. It requires a value, the rest of the string is its value
                    std::string value           = arg.substr(j + 1);
                    data.options[currentOptKey] = {i, std::move(value)};
                    isValueConsumedInCurrentArg = true;
                    break; // Stop processing this argument, as the rest is a value for this option
                }

                // It's a flag
                data.options[currentOptKey] = {-i, ""};
                // Keep track of the last flag, in case it needs to consume the next argument
                lastFlagKey = currentOptKey;
            }

            // `-n 123` or `-abn 123` form
            // If no value was consumed within the current argument (not `-n123` form)
            // and there was a last flag, check if it takes a value from the next argument.
            if (!isValueConsumedInCurrentArg && !lastFlagKey.empty()) {
                // This condition applies to the *last* flag in a bundle (e.g., 'c' in -abc)
                // or a single short option (e.g., 'a' in -a).
                // If the next argument exists and is not another option, it's the value.
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    data.options[lastFlagKey] = {i + 1, ""};
                    i++; // Consume the next argument
                }
            }
        }
        // Positional
        else {
            data.positionalArgsIndices.push_back(i);
        }
    }
}

inline void Parser::tryToPrintVersion_(InternalData &data) {
    if (programVersion_.empty()) { return; }
    data.optionHelpEntries.push_back({"-V", "--version", "Show version information and exit", ""});
    if ((data.options.count("-V") != 0) || (data.options.count("--version")) != 0) {
        std::cout << programVersion_ << '\n';
        std::exit(EXIT_SUCCESS);
    }
}

inline void Parser::tryToPrintHelp_(InternalData &data) {
    tryToPrintVersion_(data);

    if ((data.options.count("-h") != 0) || (data.options.count("--help")) != 0) {
        data.optionHelpEntries.push_back({"-h", "--help", "Show this help message and exit", ""});
        printHelp(data);
        std::exit(EXIT_SUCCESS);
    }
}

inline bool Parser::tryToPrintInvalidOpts_(InternalData &data, bool notExit) {
    // Remove help options as they are handled by tryToPrintHelp
    data.options.erase("-h");
    data.options.erase("--help");

    if (!data.options.empty()) {
        for (const auto &pair : data.options) {
            std::cerr << ERROR_STR << "Unrecognized option '";
#ifdef ARGLITE_ENABLE_FORMATTER
            std::cerr << Formatter::bold(pair.first);
#else
            std::cerr << pair.first;
#endif
            std::cerr << "'\n";
        }
        if (!notExit) { std::exit(EXIT_FAILURE); }
        return true;
    }

    return false;
}

inline void Parser::printHelp(const InternalData &data) {
    printHelpDescription(programDescription_);
    printHelpUsage(data, data_.cmdName);
    printHelpSubCmd(subCmdPtrs_);
    printHelpPositional(data);
    printHelpOptions(data);
}

inline void Parser::printHelpDescription(std::string_view description) {
    if (!description.empty()) {
        std::cout << description << '\n'
                  << '\n';
    }
}

inline void Parser::printHelpUsage(const InternalData &data, std::string_view cmdName) {
    std::cout << "Usage: ";
#ifdef ARGLITE_ENABLE_FORMATTER
    std::cout << Formatter::bold(cmdName);
#else
    std::cout << cmdName;
#endif
    if (!subCmdPtrs_.empty()) { std::cout << " [SUBCOMMAND]"; }
    if (!data.optionHelpEntries.empty()) { std::cout << " [OPTIONS]"; }
    for (const auto &p : data.positionalHelpEntries) {
        std::cout << " " << (p.required ? "" : "[") << p.name << (p.required ? "" : "]");
    }
    std::cout << '\n';
}

inline void Parser::printHelpSubCmd(const std::vector<SubParser *> &subCmdPtrs) {
    if (subCmdPtrs.empty() || !isMainCmdActive()) { return; }

#ifdef ARGLITE_ENABLE_FORMATTER
    std::cout << '\n'
              << Formatter::boldUnderline("Subcommands:") << '\n';
#else
    std::cout << "\nSubcommands:\n";
#endif

    size_t maxSubCmdNameWidth = 0;
    for (const auto &p : subCmdPtrs) {
        maxSubCmdNameWidth = std::max(maxSubCmdNameWidth, p->subCommandName_.length());
    }

    for (const auto &p : subCmdPtrs) {
        std::cout << "  " << std::left
                  << std::setw(static_cast<int>(maxSubCmdNameWidth) + 3)
                  << p->subCommandName_ << p->subCmdDescription_ << '\n';
    }
}

inline void Parser::printHelpPositional(const InternalData &data) {
    if (!data.positionalHelpEntries.empty()) {
#ifdef ARGLITE_ENABLE_FORMATTER
        std::cout << '\n'
                  << Formatter::boldUnderline("Positional Arguments:") << '\n';
#else
        std::cout << "\nPositional Arguments:\n";
#endif
        size_t maxNameWidth = 0;
        for (const auto &p : data.positionalHelpEntries) {
            maxNameWidth = std::max(maxNameWidth, p.name.length());
        }
        for (const auto &p : data.positionalHelpEntries) {
            std::cout << "  " << std::left;
#ifdef ARGLITE_ENABLE_FORMATTER
            constexpr int ANSI_CODE_LENGTH = 8; // 4 + 4 (\x1b[1m + \x1b[0m))
            std::cout << std::setw(static_cast<int>(maxNameWidth) + 2 + ANSI_CODE_LENGTH)
                      << Formatter::bold(p.name);
#else
            std::cout << std::setw(static_cast<int>(maxNameWidth) + 2) << p.name;
#endif
            std::cout << p.description << '\n';
        }
    }
}

inline void Parser::printHelpOptions(const InternalData &data) {
    if (!data.optionHelpEntries.empty()) {
#ifdef ARGLITE_ENABLE_FORMATTER
        std::cout << '\n'
                  << Formatter::boldUnderline("Options:") << '\n';
#else
        std::cout << "\nOptions:\n";
#endif

        for (const auto &o : data.optionHelpEntries) {
            std::string optStr("  ");
            if (!o.shortOpt.empty()) {
                optStr += o.shortOpt;
                if (!o.longOpt.empty()) { optStr += ", "; }
            } else {
                optStr += "    "; // Pad for alignment
            }
            optStr += o.longOpt;

            std::cout << std::left;
#ifdef ARGLITE_ENABLE_FORMATTER
            constexpr int ANSI_CODE_LENGTH = 8; // 4 + 4 (\x1b[1m + \x1b[0m))
            std::cout << std::setw(static_cast<int>(descriptionIndent_) + ANSI_CODE_LENGTH);
            optStr = Formatter::bold(optStr);
#else
            std::cout << std::setw(static_cast<int>(descriptionIndent_));
#endif

            if (!o.typeName.empty()) {
                optStr.append(" <").append(o.typeName).append(">");
            }
            std::cout << optStr;

            std::string descStr = o.description;
            if (!o.defaultValue.empty()) {
                descStr += std::string(" [default: ").append(o.defaultValue).append("]");
            }
            // the option string is too long, start a new line
            // -2: two separeting spaces after the type name
            auto optPartLength = optStr.length();
#ifdef ARGLITE_ENABLE_FORMATTER
            optPartLength -= ANSI_CODE_LENGTH;
#endif
            if (optPartLength > descriptionIndent_ - 2) {
                std::cout << '\n'
                          << std::left << std::setw(static_cast<int>(descriptionIndent_)) << "";
            }
            std::cout << descStr << '\n';
        }
    }
}

// Clear internal data
inline void Parser::clearData(InternalData &data) {
    InternalData temp;
    temp.options.swap(data.options);
    temp.optionHelpEntries.swap(data.optionHelpEntries);
    temp.positionalArgsIndices.swap(data.positionalArgsIndices);
    temp.positionalHelpEntries.swap(data.positionalHelpEntries);
    temp.errorMessages.swap(data.errorMessages);
}

inline bool Parser::finalize_(InternalData &data, bool notExit) {
    if (data.errorMessages.empty()) {
        clearData(data);
        return false;
    }

    std::cerr << "Errors occurred while parsing command-line arguments.\n";
    std::cerr << "The following is a list of error messages:\n";
    for (const auto &msg : data.errorMessages) {
        std::cerr << ERROR_STR << msg << '\n';
    }

    if (notExit) {
        clearData(data);
        return true;
    }
    std::exit(EXIT_FAILURE);
}

inline bool Parser::runAllPostprocess_(InternalData &data, bool notExit) {
    tryToPrintHelp_(data);
    auto hasInvalidOpts = tryToPrintInvalidOpts_(data, true);
    auto hasError       = finalize_(data, true);

    if (!notExit && (hasInvalidOpts || hasError)) {
        std::exit(EXIT_FAILURE);
    }
    return hasInvalidOpts || hasError;
}

} // namespace ArgLite

#include "Get.hpp" // IWYU pragma: keep
