#pragma once

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace ArgLite {

class Parser {
public:
    /**
     * @brief Sets the program description, used for the first line of the help message.
     * @param description The program's description text.
     */
    static inline void setDescription(const std::string &description);

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
    static inline bool hasFlag(const std::string &optName, const std::string &description);

    /**
     * @brief Gets the value of a string option.
     * @param names Option names (e.g., "o", "output" or "o,output").
     * @param description Option description.
     * @param defaultValue The default value to return if the option is not provided on the command line.
     * @return The parsed string value or the default value.
     */
    static inline std::string getString(const std::string &optName, const std::string &description, const std::string &defaultValue = "");

    /**
     * @brief Gets the value of an integer option.
     * @param names Option names (e.g., "n", "count" or "n,count").
     * @param description Option description.
     * @param defaultValue The default value.
     * @return The parsed integer value or the default value. If the provided value cannot be converted to an integer, the program will report an error and exit.
     */
    static inline long long getInt(const std::string &optName, const std::string &description, long long defaultValue = 0);

    /**
     * @brief Gets the value of a floating-point option.
     * @param names Option names (e.g., "r", "rate" or "r,rate").
     * @param description Option description.
     * @param defaultValue The default value.
     * @return The parsed floating-point value or the default value. If the provided value cannot be converted to a floating-point number, the program will report an error and exit.
     */
    static inline double getDouble(const std::string &optName, const std::string &description, double defaultValue = 0.0);

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
    static inline bool getBool(const std::string &optName, const std::string &description, bool defaultValue = false);

    /**
     * @brief Gets a positional argument.
     * @details Must be called after all get/hasFlag calls. Should be called in order.
     * @param name Argument name, used for the help message (e.g., "input-file").
     * @param description Argument description.
     * @param required If true and the user does not provide the argument, the program will report an error and exit.
     * @return The string value of the argument. If the argument is not required and not provided, returns an empty string.
     */
    static inline std::string getPositional(const std::string &name, const std::string &description, bool required = true);

    /**
     * @brief Gets all remaining positional arguments.
     * @details Must be called after all getPositional calls.
     * @param name Argument name, used for the help message (e.g., "extra-files").
     * @param description Argument description.
     * @param required If true and there are no remaining arguments, the program will report an error and exit.
     * @return A string vector containing all remaining arguments.
     */
    static inline std::vector<std::string> getRemainingPositionals(const std::string &name, const std::string &description, bool required = true);

    /**
     * @brief If the user provides -h or --help, prints the help message and exits the program normally.
     * @details Recommended to be called after all get/hasFlag calls, and before tryToPrintInvalidOpts.
     */
    static inline void tryToPrintHelp();

    /**
     * @brief Checks and reports all unknown options that were not processed by get/hasFlag.
     * @details If unknown options exist, prints an error message and exits the program abnormally.
     *          This function should be called after all argument retrieval function calls.
     */
    static inline void tryToPrintInvalidOpts();

private:
    struct OptionHelpInfo {
        std::string shortOpt;
        std::string longOpt;
        std::string description;
        std::string defaultValue;
    };

    struct PositionalHelpInfo {
        std::string name;
        std::string description;
        bool        required;
    };

    // Stores option information for subsequent get/hasFlag calls.
    // key: Option name (e.g., "-o", "--output").
    // value: index > 0: Index of the argument in argv;
    // index < 0: Index of the flag option in argv;
    // index == 0: Default value, usually indicating --opt=val form.
    struct OptionInfo {
        int         argvIndex;
        std::string valueFromEquals; // Only used for --opt=val form
    };

    // Internal data storage
    static inline std::string programDescription_;
    static inline std::string programName_;
    static inline int         argc_          = 0;
    static inline char      **argv_          = nullptr;
    static inline size_t      positionalIdx_ = 0;
    // Containers
    static inline std::unordered_map<std::string, OptionInfo> options_;
    static inline std::vector<OptionHelpInfo>                 optionHelpEntries_;
    static inline std::vector<int>                            positionalArgsIndices_;
    static inline std::vector<PositionalHelpInfo>             positionalHelpEntries_;

    // Internal helper functions
    // Get functions, containers can be changed
    static inline bool                     hasFlag_(const std::string &optName, const std::string &description, std::unordered_map<std::string, OptionInfo> &options = options_, std::vector<OptionHelpInfo> &optionHelpEntries = optionHelpEntries_, std::vector<int> &positionalArgsIndices = positionalArgsIndices_);
    static inline std::string              getString_(const std::string &optName, const std::string &description, const std::string &defaultValue = "", std::unordered_map<std::string, OptionInfo> &options = options_, std::vector<OptionHelpInfo> &optionHelpEntries = optionHelpEntries_);
    static inline long long                getInt_(const std::string &optName, const std::string &description, long long defaultValue = 0, std::unordered_map<std::string, OptionInfo> &options = options_, std::vector<OptionHelpInfo> &optionHelpEntries = optionHelpEntries_);
    static inline double                   getDouble_(const std::string &optName, const std::string &description, double defaultValue = 0.0, std::unordered_map<std::string, OptionInfo> &options = options_, std::vector<OptionHelpInfo> &optionHelpEntries = optionHelpEntries_);
    static inline bool                     getBool_(const std::string &optName, const std::string &description, bool defaultValue = false, std::unordered_map<std::string, OptionInfo> &options = options_, std::vector<OptionHelpInfo> &optionHelpEntries = optionHelpEntries_);
    static inline std::string              getPositional_(const std::string &name, const std::string &description, bool required = true, std::vector<int> &positionalArgsIndices = positionalArgsIndices_, std::vector<PositionalHelpInfo> &positionalHelpEntries = positionalHelpEntries_);
    static inline std::vector<std::string> getRemainingPositionals_(const std::string &name, const std::string &description, bool isRequired = true, std::vector<int> &positionalArgsIndices = positionalArgsIndices_, std::vector<PositionalHelpInfo> &positionalHelpEntries = positionalHelpEntries_);
    // Other functions
    static inline void                         printErrorAndExit(const std::string &message);
    static inline void                         parseOptName(const std::string &names, std::string &shortOpt, std::string &longOpt);
    static inline std::pair<bool, OptionInfo>  findOption(const std::string &shortOpt, const std::string &longOpt);
    static inline std::pair<bool, std::string> getValueStr(const std::string &optName, const std::string &description, const std::string &defaultValueStr);
    static inline void                         printHelp();
    template <typename T>
    static inline std::string T_to_string(const T &val);
};

// ========================================================================
// Method Definitions
// ========================================================================
inline void Parser::setDescription(const std::string &description) {
    programDescription_ = description;
}

inline void Parser::preprocess(int argc, char **argv) {
    argc_ = argc;
    argv_ = argv;
    if (argc > 0) {
        programName_ = argv[0];
        // Extract the basename
        size_t last_slash_pos = programName_.find_last_of("/\\");
        if (std::string::npos != last_slash_pos) {
            programName_.erase(0, last_slash_pos + 1);
        }
    }

    bool allPositional = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (allPositional) {
            positionalArgsIndices_.push_back(i);
            continue;
        }

        if (arg == "--") {
            allPositional = true;
            continue;
        }

        if (arg.rfind("--", 0) == 0) { // Long option
            std::string key = arg;
            std::string value;
            size_t      equalsPos = arg.find('=');
            if (equalsPos != std::string::npos) { // --opt=val form
                key           = arg.substr(0, equalsPos);
                value         = arg.substr(equalsPos + 1);
                options_[key] = {0, value};
            } else {
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    options_[key] = {i + 1, ""};
                    i++; // Consume next arg as value
                } else {
                    options_[key] = {-i, ""}; // Flag
                }
            }
        } else if (arg.rfind("-", 0) == 0) { // Short option(s)
            if (arg.length() > 2) {          // Bundled flags, e.g., -abc
                for (size_t j = 1; j < arg.length(); ++j) {
                    std::string key = "-";
                    key += arg[j];
                    if (j == arg.length() - 1) { // Last char might have a value
                        if (i + 1 < argc && argv[i + 1][0] != '-') {
                            options_[key] = {i + 1, ""};
                            i++;
                        } else {
                            options_[key] = {-i, ""}; // Flag
                        }
                    } else {
                        options_[key] = {-i, ""}; // All but the last are flags
                    }
                }
            } else { // Single short option, e.g., -a
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    options_[arg] = {i + 1, ""};
                    i++;
                } else {
                    options_[arg] = {-i, ""}; // Flag
                }
            }
        } else { // Positional
            positionalArgsIndices_.push_back(i);
        }
    }
}

inline bool Parser::hasFlag(const std::string &optName, const std::string &description) {
    return hasFlag_(optName, description);
}

inline std::string Parser::getString(const std::string &optName, const std::string &description, const std::string &defaultValue) {
    return getString_(optName, description, defaultValue);
}

inline long long Parser::getInt(const std::string &optName, const std::string &description, long long defaultValue) {
    return Parser::getInt_(optName, description, defaultValue);
}

inline double Parser::getDouble(const std::string &optName, const std::string &description, double defaultValue) {
    return getDouble_(optName, description, defaultValue);
}

inline bool Parser::getBool(const std::string &optName, const std::string &description, bool defaultValue) {
    return getBool_(optName, description, defaultValue);
}

inline std::string Parser::getPositional(const std::string &name, const std::string &description, bool isRequired) {
    return getPositional_(name, description, isRequired);
}

inline std::vector<std::string> Parser::getRemainingPositionals(const std::string &name, const std::string &description, bool required) {
    return getRemainingPositionals_(name, description, required);
}

inline void Parser::tryToPrintHelp() {
    if (options_.count("-h") || options_.count("--help")) {
        optionHelpEntries_.push_back({"-h", "--help", "Show this help message and exit", ""});
        printHelp();
        exit(0);
    }
}

inline void Parser::tryToPrintInvalidOpts() {
    // Remove help options as they are handled by tryToPrintHelp
    options_.erase("-h");
    options_.erase("--help");

    if (!options_.empty()) {
        for (const auto &pair : options_) {
            std::cerr << "Error: Unrecognized option '" << pair.first << "'" << '\n';
        }
        exit(1);
    }
}

// === Private Helper Implementations ===

inline bool Parser::hasFlag_(
    const std::string &optName, const std::string &description,
    std::unordered_map<std::string, OptionInfo> &options,
    std::vector<OptionHelpInfo>                 &optionHelpEntries,
    std::vector<int>                            &positionalArgsIndices) {

    std::string shortOpt;
    std::string longOpt;
    parseOptName(optName, shortOpt, longOpt);
    optionHelpEntries.push_back({shortOpt, longOpt, description, ""});

    auto [found, info] = findOption(shortOpt, longOpt);

    if (found) {
        // A flag was passed with a value, e.g., -f 123. The value is likely a positional arg.
        if (info.argvIndex > 0) {
            positionalArgsIndices.push_back(info.argvIndex);
            // Keep positional args sorted by their original index to maintain order
            std::sort(positionalArgsIndices.begin(), positionalArgsIndices.end());
        }
        if (!shortOpt.empty()) options.erase(shortOpt);
        if (!longOpt.empty()) options.erase(longOpt);
    }

    return found;
}

inline std::string Parser::getString_(
    const std::string &optName, const std::string &description, const std::string &defaultValue,
    std::unordered_map<std::string, OptionInfo> &options,
    std::vector<OptionHelpInfo>                 &optionHelpEntries) {

    auto [found, value] = getValueStr(optName, description, defaultValue);
    return value;
}

inline long long Parser::getInt_(
    const std::string &optName, const std::string &description, long long defaultValue,
    std::unordered_map<std::string, OptionInfo> &options,
    std::vector<OptionHelpInfo>                 &optionHelpEntries) {

    auto [found, valueStr] = getValueStr(optName, description, T_to_string(defaultValue));
    if (!found) {
        return defaultValue;
    }
    try {
        return std::stoll(valueStr);
    } catch (const std::exception &) {
        std::string shortOpt;
        std::string longOpt;
        parseOptName(optName, shortOpt, longOpt);
        std::string optName = !longOpt.empty() ? longOpt : shortOpt;
        printErrorAndExit("Invalid value for option '" + optName + "'. Expected an integer, but got '" + valueStr + "'.");
    }
    return 0; // Should not reach here
}

inline double Parser::getDouble_(
    const std::string &optName, const std::string &description, double defaultValue,
    std::unordered_map<std::string, OptionInfo> &options,
    std::vector<OptionHelpInfo>                 &optionHelpEntries) {

    auto [found, valueStr] = getValueStr(optName, description, T_to_string(defaultValue));
    if (!found) {
        return defaultValue;
    }
    try {
        return std::stod(valueStr);
    } catch (const std::exception &) {
        std::string shortOpt;
        std::string longOpt;
        parseOptName(optName, shortOpt, longOpt);
        std::string optName = !longOpt.empty() ? longOpt : shortOpt;
        printErrorAndExit("Invalid value for option '" + optName + "'. Expected a number, but got '" + valueStr + "'.");
    }
    return 0.0; // Should not reach here
}

inline bool Parser::getBool_(
    const std::string &optName, const std::string &description, bool defaultValue,
    std::unordered_map<std::string, OptionInfo> &options,
    std::vector<OptionHelpInfo>                 &optionHelpEntries) {

    auto [found, valueStr] = getValueStr(optName, description, defaultValue ? "true" : "false");
    if (!found) {
        return defaultValue;
    }

    std::transform(valueStr.begin(), valueStr.end(), valueStr.begin(), ::tolower);
    if (valueStr == "true" || valueStr == "1" || valueStr == "yes" || valueStr == "on") {
        return true;
    }
    if (valueStr == "false" || valueStr == "0" || valueStr == "no" || valueStr == "off") {
        return false;
    }

    std::string shortOpt;
    std::string longOpt;
    parseOptName(optName, shortOpt, longOpt);
    std::string opt = !longOpt.empty() ? longOpt : shortOpt;
    printErrorAndExit("Invalid value for option '" + opt + "'. Expected a boolean, but got '" + valueStr + "'.");
    return false; // Should not reach here
}

inline std::string Parser::getPositional_(
    const std::string &name, const std::string &description, bool isRequired,
    std::vector<int>                &positionalArgsIndices,
    std::vector<PositionalHelpInfo> &positionalHelpEntries) {

    positionalHelpEntries_.push_back({name, description, isRequired});
    if (positionalIdx_ < positionalArgsIndices_.size()) {
        int argv_idx = positionalArgsIndices_[positionalIdx_];
        positionalIdx_++;
        return argv_[argv_idx];
    }
    if (isRequired) {
        printErrorAndExit("Missing required positional argument '" + name + "'.");
    }
    return "";
}

inline std::vector<std::string> Parser::getRemainingPositionals_(
    const std::string &name, const std::string &description, bool required,
    std::vector<int>                &positionalArgsIndices,
    std::vector<PositionalHelpInfo> &positionalHelpEntries) {

    positionalHelpEntries_.push_back({name, description, required});
    std::vector<std::string> remaining;
    while (positionalIdx_ < positionalArgsIndices_.size()) {
        int argv_idx = positionalArgsIndices_[positionalIdx_];
        remaining.push_back(argv_[argv_idx]);
        positionalIdx_++;
    }
    if (required && remaining.empty()) {
        printErrorAndExit("Missing required positional argument(s) '" + name + "'.");
    }
    return remaining;
}

inline void Parser::printErrorAndExit(const std::string &message) {
    std::cerr << "Error: " << message << '\n';
    exit(1);
}

inline void Parser::parseOptName(const std::string &names, std::string &shortOpt, std::string &longOpt) {
    size_t commaPos = names.find(',');
    if (commaPos == std::string::npos) {
        if (names.length() > 1) longOpt = "--" + names;
        else shortOpt = "-" + names;
    } else {
        shortOpt = "-" + names.substr(0, commaPos);
        longOpt  = "--" + names.substr(commaPos + 1);
    }
}

inline std::pair<bool, Parser::OptionInfo> Parser::findOption(const std::string &shortOpt, const std::string &longOpt) {
    auto longIt  = longOpt.empty() ? options_.end() : options_.find(longOpt);
    auto shortIt = shortOpt.empty() ? options_.end() : options_.find(shortOpt);

    bool longFound  = longIt != options_.end();
    bool shortFound = shortIt != options_.end();

    if (!longFound && !shortFound) {
        return {false, {0, ""}};
    }

    // If both are found, prefer the one that appears later in argv
    if (longFound && shortFound) {
        if (std::abs(longIt->second.argvIndex) > std::abs(shortIt->second.argvIndex)) {
            return {true, longIt->second};
        }
        return {true, shortIt->second};
    }

    if (longFound) return {true, longIt->second};
    return {true, shortIt->second};
}

inline std::pair<bool, std::string> Parser::getValueStr(const std::string &optName, const std::string &description, const std::string &defaultValueStr) {
    std::string shortOpt;
    std::string longOpt;
    parseOptName(optName, shortOpt, longOpt);
    optionHelpEntries_.push_back({shortOpt, longOpt, description, defaultValueStr});

    auto [found, optInfo] = findOption(shortOpt, longOpt);

    if (found) {
        if (!shortOpt.empty()) options_.erase(shortOpt);
        if (!longOpt.empty()) options_.erase(longOpt);

        if (optInfo.argvIndex < 0) { // It's a flag but a value was expected
            printErrorAndExit("Option '" + (!longOpt.empty() ? longOpt : shortOpt) + "' does not take a value.");
        }
        if (optInfo.argvIndex == 0) { // From --opt=val
            return {true, optInfo.valueFromEquals};
        }
        return {true, argv_[optInfo.argvIndex]};
    }

    return {false, defaultValueStr};
}

inline void Parser::printHelp() {
    if (!programDescription_.empty()) {
        std::cout << programDescription_ << '\n'
                  << '\n';
    }

    // Usage line
    std::cout << "Usage: " << programName_;
    if (!optionHelpEntries_.empty()) std::cout << " [OPTIONS]";
    for (const auto &p : positionalHelpEntries_) {
        std::cout << " " << (p.required ? "" : "[") << p.name << (p.required ? "" : "]");
    }
    std::cout << '\n';

    // Positional Arguments
    if (!positionalHelpEntries_.empty()) {
        std::cout << "\nPositional Arguments:" << '\n';
        size_t maxNameWidth = 0;
        for (const auto &p : positionalHelpEntries_) {
            if (p.name.length() > maxNameWidth) {
                maxNameWidth = p.name.length();
            }
        }
        for (const auto &p : positionalHelpEntries_) {
            std::cout << "  " << std::left << std::setw(maxNameWidth + 2) << p.name << p.description << '\n';
        }
    }

    // Options
    if (!optionHelpEntries_.empty()) {
        std::cout << "\nOptions:" << '\n';
        size_t maxOptWidth = 0;
        for (const auto &o : optionHelpEntries_) {
            size_t currentWidth = 0;
            if (!o.shortOpt.empty()) currentWidth += o.shortOpt.length();
            if (!o.longOpt.empty()) currentWidth += o.longOpt.length();
            if (!o.shortOpt.empty() && !o.longOpt.empty()) currentWidth += 2; // for ", "
            if (currentWidth > maxOptWidth) {
                maxOptWidth = currentWidth;
            }
        }
        maxOptWidth += 4; // for padding around short-only or long-only options
        const size_t descriptionIndent = 25;

        for (const auto &o : optionHelpEntries_) {
            std::stringstream ss;
            ss << "  ";
            if (!o.shortOpt.empty()) {
                ss << o.shortOpt;
                if (!o.longOpt.empty()) ss << ", ";
            } else {
                ss << "    "; // Pad for alignment
            }
            ss << o.longOpt;

            std::string optStr = ss.str();
            std::cout << std::left << std::setw(descriptionIndent) << optStr;

            std::string desc = o.description;
            if (!o.defaultValue.empty()) {
                desc += " [default: " + o.defaultValue + "]";
            }
            std::cout << desc << '\n';
        }
    }
}

template <typename T>
inline std::string Parser::T_to_string(const T &val) {
    std::stringstream ss;
    ss << val;
    return ss.str();
}

} // namespace ArgLite
