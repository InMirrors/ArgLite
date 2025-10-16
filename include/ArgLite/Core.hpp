#pragma once

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

class Parser {
public:
    /**
     * @brief Sets the program description, used for the first line of the help message.
     * @param description The program's description text.
     */
    static inline void setDescription(std::string_view description);

    /**
     * @brief Sets the program version and add options `-V` and `--version` to print the version.
     * @param versionStr The program's version string.
     */
    static inline void setVersion(std::string_view versionStr);

    /**
    * @brief Sets which short options require a value.
     * @details To pass short options like `-n 123` as a single argument `-n123`,
                provide the short option names as a string to this function.
                You don't have to call it, but if you do, call it before `preprocess()`.
     * @param shortNonFlagOptsStr A string containing all short option characters that require a value.
                                  For example, if `-n` and `-r` require values, pass `nr`.
     */
    static inline void setShortNonFlagOptsStr(std::string_view shortNonFlagOptsStr);

    /**
     * @brief Preprocesses the command-line arguments. This is the first step in using this library.
     * @param argc The argc from the main function.
     * @param argv The argv from the main function.
     */
    static inline void preprocess(int argc, char **argv);

    /**
     * @brief Checks if a flag option exists.
     * @param names Option names (e.g., "v", "verbose" or "v,verbose").
     * @param description Option description, used for the help message.
     * @return Returns true if the option appears in the command line, false otherwise.
     */
    static inline bool hasFlag(std::string_view optName, const std::string &description);

    //  Structure for arguments of mutually exclusive flag options.
    struct GetMutualExArgs {
        std::string trueOptName;      // Name of the option that represents the true condition.
        std::string trueDescription;  // Description of the option that represents the true condition.
        std::string falseOptName;     // Name of the option that represents the false condition.
        std::string falseDescription; // Description of the option that represents the false condition.
        bool        defaultValue;     // Default value if neither option is specified.
    };

    /**
     * @brief Checks if two mutually exclusive options exist.
     * @param args Structure containing the names and descriptions of the mutually exclusive options.
     * @return True if the first option is present and the second is not, or vice versa;
               defaultValue if neither option is present.
     */
    static inline bool hasMutualExFlag(const GetMutualExArgs &args);

    /**
     * @brief Gets the value of a string option.
     * @param names Option names (e.g., "o", "output" or "o,output").
     * @param description Option description.
     * @param defaultValue The default value to return if the option is not provided on the command line.
     * @return The parsed string value or the default value.
     */
    static inline std::string getString(std::string_view optName, const std::string &description, const std::string &defaultValue = "");

    /**
     * @brief Gets the value of an integer option.
     * @param names Option names (e.g., "n", "count" or "n,count").
     * @param description Option description.
     * @param defaultValue The default value.
     * @return The parsed integer value or the default value.
     */
    static inline long long getInt(std::string_view optName, const std::string &description, long long defaultValue = 0);

    /**
     * @brief Gets the value of a floating-point option.
     * @param names Option names (e.g., "r", "rate" or "r,rate").
     * @param description Option description.
     * @param defaultValue The default value.
     * @return The parsed floating-point value or the default value.
     */
    static inline double getDouble(std::string_view optName, const std::string &description, double defaultValue = 0.0);

    /**
     * @brief Gets the value of a boolean option.
     * @details "1", "true", "yes", "on" (case-insensitive) will be parsed as true.
     *          "0", "false", "no", "off" (case-insensitive) will be parsed as false.
     *          Other values will cause the program to report an error and exit.
     * @param names Option names (e.g., "e", "enable" or "e,enable").
     * @param description Option description.
     * @param defaultValue The default value.
     * @return The parsed boolean value or the default value.
     */
    static inline bool getBool(std::string_view optName, const std::string &description, bool defaultValue = false);

    /**
     * @brief Gets a positional argument.
     * @details Must be called after all get/hasFlag calls. Should be called in order.
     * @param name Argument name, used for the help message (e.g., "input-file").
     * @param description Argument description.
     * @param required If true and the user does not provide the argument, the program will report an error and exit.
     * @return The string value of the argument. If the argument is not required and not provided, returns an empty string.
     */
    static inline std::string getPositional(const std::string &posName, const std::string &description, bool required = true);

    /**
     * @brief Gets all remaining positional arguments.
     * @details Must be called after all getPositional calls.
     * @param name Argument name, used for the help message (e.g., "extra-files").
     * @param description Argument description.
     * @param required If true and there are no remaining arguments, the program will report an error and exit.
     * @return A string vector containing all remaining arguments.
     */
    static inline std::vector<std::string> getRemainingPositionals(const std::string &posName, const std::string &description, bool required = true);

    /**
     * @brief Changes the description indent of option descriptions in the help message. Default is 25.
     * @details This function should be called before tryToPrintHelp.
     * @param indent The new description indent.
     */
    static inline void changeDescriptionIndent(size_t indent);

    /**
     * @brief If the user provides -h or --help, prints the help message and exits the program normally.
     * @details Recommended to be called after all get/hasFlag calls, and before tryToPrintInvalidOpts.
     */
    static inline void tryToPrintHelp();

    /**
     * @brief Checks and reports all unknown options that were not processed by get/hasFlag.
     * @details If unknown options exist, prints an error message and exits the program abnormally.
     *          This function should be called after all argument retrieval function calls.
     * @param notExit If true, the program will not exit after printing any invalid options. Default is false.
     * @return Returns true if there are any unknown options and notExit is false, false otherwise.
     */
    static inline bool tryToPrintInvalidOpts(bool notExit = false);

    /**
     * @brief Finalizes the parser. This function should be called at the end of parsing.
     * @details This function will print error messages and exit the program if there are any and notExit is true.
     * @param notExit If true, the program will not exit after printing any error messages. Default is false.
     * @return Returns true if there are any error messages and notExit is false, false otherwise.
     */
    static inline bool finalize(bool notExit = false);

    /**
     * @brief Runs tryToPrintHelp, tryToPrintInvalidOpts, and finalize in sequence.
     * @param notExit If true, the program will not exit after printing any invalid options
                      or any error messages. Default is false.
     * @return Returns true if there are any invalid options or any error messages and notExit is false,
               false otherwise.
     */
    static inline bool runAllPostprocess(bool notExit = false);

    Parser() = delete;

private:
    // Stores option information for subsequent get/hasFlag calls.
    // key: Option name (e.g., "-o", "--output").
    // value: index > 0: Index of the argument in argv;
    // index < 0: Index of the flag option in argv;
    // index == 0: Default value, no special meaning yet.
    struct OptionInfo {
        int         argvIndex;
        std::string valueStr; // Only used for -n123 and --opt=val forms
    };

    struct OptionHelpInfo {
        std::string shortOpt; // Prepended with "-"
        std::string longOpt;  // Prepended with "--"
        std::string description;
        std::string defaultValue;
    };

    struct PositionalHelpInfo {
        std::string name;
        std::string description;
        bool        required;
    };

    using OptMap = std::unordered_map<std::string, OptionInfo>;

    struct InternalData {
        std::string programName;
        std::string programDescription;
        std::string shortNonFlagOptsStr;
        // Containers
        OptMap                          options;
        std::vector<OptionHelpInfo>     optionHelpEntries;
        std::vector<int>                positionalArgsIndices;
        std::vector<PositionalHelpInfo> positionalHelpEntries;
        std::vector<std::string>        errorMessages;
    };

    // Internal data storage
    static inline int          argc_;
    static inline char       **argv_;
    static inline size_t       positionalIdx_;
    static inline size_t       descriptionIndent_ = 25; // NOLINT(readability-magic-numbers)
    static inline std::string  programVersion_;
    static inline InternalData data_;

    // Internal helper functions
    // Get functions, internal data can be changed
    static inline bool                     hasFlag_(std::string_view optName, const std::string &description, InternalData &data = data_);
    static inline bool                     hasMutualExFlag_(const GetMutualExArgs &args, InternalData &data = data_);
    static inline std::string              getString_(std::string_view optName, const std::string &description, const std::string &defaultValue = "", InternalData &data = data_);
    static inline long long                getInt_(std::string_view optName, const std::string &description, long long defaultValue = 0, InternalData &data = data_);
    static inline double                   getDouble_(std::string_view optName, const std::string &description, double defaultValue = 0.0, InternalData &data = data_);
    static inline bool                     getBool_(std::string_view optName, const std::string &description, bool defaultValue = false, InternalData &data = data_);
    static inline std::string              getPositional_(const std::string &posName, const std::string &description, bool required = true, InternalData &data = data_);
    static inline std::vector<std::string> getRemainingPositionals_(const std::string &posName, const std::string &description, bool isRequired = true, InternalData &data = data_);
    // Helper functions for get functions
    static inline void appendOptValErrorMsg(InternalData &data, std::string_view optName, const std::string &typeName, const std::string &valueStr);
    static inline void appendPosValErrorMsg(InternalData &data, std::string_view posName, std::string_view errorMsg);
    static inline void fixPositionalArgsArray(std::vector<int> &positionalArgsIndices, std::unordered_map<std::string, OptionInfo> &options);
    template <typename T>
    static inline std::string toString(const T &val);
    // Helper functions for get functions with long return types
    static inline std::string                                            parseOptName(std::string_view optName);
    static inline std::pair<std::string, std::string>                    parseOptNameAsPair(std::string_view optName);
    static inline std::unordered_map<std::string, OptionInfo>::node_type findOption(const std::string &shortOpt, const std::string &longOpt, InternalData &data);
    static inline std::pair<bool, std::string>                           getValueStr(std::string_view optName, const std::string &description, const std::string &defaultValueStr, InternalData &data);
    // Other helper functions
    static inline void preprocess_(int argc, char **argv, InternalData &data = data_);
    static inline void tryToPrintVersion_(InternalData &data = data_);
    static inline void tryToPrintHelp_(InternalData &data = data_);
    static inline bool tryToPrintInvalidOpts_(InternalData &data = data_, bool notExit = false);
    static inline void printHelp(const InternalData &data = data_);
    static inline void printHelpDescription(std::string_view description);
    static inline void printHelpUsage(const InternalData &data, std::string_view cmdName);
    static inline void printHelpPositional(const InternalData &data);
    static inline void printHelpOptions(const InternalData &data);
    static inline bool finalize_(const std::vector<std::string> &errorMessages, bool notExit = false);
    static inline bool runAllPostprocess_(InternalData &data, bool notExit = false);

#ifdef ARGLITE_ENABLE_FORMATTER
    static inline const std::string ERROR_STR = Formatter::red("Error: ");
#else
    static inline const std::string ERROR_STR = "Error: ";
#endif
};

// ========================================================================
// Method Definitions
// ========================================================================
inline void Parser::setDescription(std::string_view description) {
    data_.programDescription = description;
}

inline void Parser::setVersion(std::string_view versionStr) { programVersion_ = versionStr; }

inline void Parser::setShortNonFlagOptsStr(std::string_view shortNonFlagOptsStr) {
    data_.shortNonFlagOptsStr = shortNonFlagOptsStr;
}

inline void Parser::preprocess(int argc, char **argv) {
    preprocess_(argc, argv);
}

inline bool Parser::hasFlag(std::string_view optName, const std::string &description) {
    return hasFlag_(optName, description);
}

inline bool Parser::hasMutualExFlag(const GetMutualExArgs &args) {
    return hasMutualExFlag_(args);
}

inline std::string Parser::getString(std::string_view optName, const std::string &description, const std::string &defaultValue) {
    return getString_(optName, description, defaultValue);
}

inline long long Parser::getInt(std::string_view optName, const std::string &description, long long defaultValue) {
    return Parser::getInt_(optName, description, defaultValue);
}

inline double Parser::getDouble(std::string_view optName, const std::string &description, double defaultValue) {
    return getDouble_(optName, description, defaultValue);
}

inline bool Parser::getBool(std::string_view optName, const std::string &description, bool defaultValue) {
    return getBool_(optName, description, defaultValue);
}

inline std::string Parser::getPositional(const std::string &posName, const std::string &description, bool isRequired) {
    return getPositional_(posName, description, isRequired);
}

inline std::vector<std::string> Parser::getRemainingPositionals(const std::string &posName, const std::string &description, bool required) {
    return getRemainingPositionals_(posName, description, required);
}

inline void Parser::changeDescriptionIndent(size_t indent) { descriptionIndent_ = indent; }

inline void Parser::tryToPrintHelp() { tryToPrintHelp_(); }

inline bool Parser::tryToPrintInvalidOpts(bool notExit) { return tryToPrintInvalidOpts_(data_, notExit); }

inline bool Parser::finalize(bool notExit) { return finalize_(data_.errorMessages, notExit); }

inline bool Parser::runAllPostprocess(bool notExit) { return runAllPostprocess_(data_, notExit); }

// === Private Helper Implementations ===

inline void Parser::preprocess_(int argc, char **argv, InternalData &data) { // NOLINT(readability-function-cognitive-complexity)
    argc_ = argc;
    argv_ = argv;
    if (argc_ > 0) {
        data_.programName = argv[0];
        // Extract the basename
        size_t last_slash_pos = data_.programName.find_last_of("/\\");
        if (std::string::npos != last_slash_pos) {
            data_.programName.erase(0, last_slash_pos + 1);
        }
    }

    bool allPositional = false;
    for (int i = 1; i < argc; ++i) {
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
            size_t      equalsPos = arg.find('=');
            if (equalsPos != std::string::npos) { // --opt=val form
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
                if (data.shortNonFlagOptsStr.find(arg[j]) != std::string::npos && j + 1 < arg.length()) {
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
        printHelp();
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
    printHelpDescription(data_.programDescription);
    printHelpUsage(data, data_.programName);
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
    if (!data.optionHelpEntries.empty()) std::cout << " [OPTIONS]";
    for (const auto &p : data.positionalHelpEntries) {
        std::cout << " " << (p.required ? "" : "[") << p.name << (p.required ? "" : "]");
    }
    std::cout << '\n';
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
            std::cout << std::setw(static_cast<int>(descriptionIndent_) + ANSI_CODE_LENGTH)
                      << Formatter::bold(optStr);
#else
            std::cout << std::setw(static_cast<int>(descriptionIndent_)) << optStr;
#endif

            std::string descStr = o.description;
            if (!o.defaultValue.empty()) {
                descStr += std::string(" [default: ").append(o.defaultValue).append("]");
            }
            if (optStr.length() > descriptionIndent_ - 2) { // the option string is too long, start a new line
                std::cout << '\n'
                          << std::left << std::setw(static_cast<int>(descriptionIndent_)) << "";
            }
            std::cout << descStr << '\n';
        }
    }
}

inline bool Parser::finalize_(const std::vector<std::string> &errorMessages, bool notExit) {
    if (errorMessages.empty()) { return false; }

    std::cerr << "Errors occurred while parsing command-line arguments.\n";
    std::cerr << "The following is a list of error messages:\n";
    for (const auto &msg : errorMessages) {
        std::cerr << ERROR_STR << msg << '\n';
    }

    if (notExit) { return true; }
    std::exit(EXIT_FAILURE);
}

inline bool Parser::runAllPostprocess_(InternalData &data, bool notExit) {
    tryToPrintHelp_(data);
    auto hasInvalidOpts = tryToPrintInvalidOpts_(data, true);
    auto hasError       = finalize_(data.errorMessages, true);

    if (!notExit && (hasInvalidOpts || hasError)) {
        std::exit(EXIT_FAILURE);
    }
    return hasInvalidOpts || hasError;
}

} // namespace ArgLite

#include "Get.hpp" // IWYU pragma: keep
