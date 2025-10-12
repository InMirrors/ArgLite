#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
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
    static inline bool hasFlag(std::string_view optName, const std::string &description);

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
     * @return The parsed integer value or the default value. If the provided value cannot be converted to an integer, the program will report an error and exit.
     */
    static inline long long getInt(std::string_view optName, const std::string &description, long long defaultValue = 0);

    /**
     * @brief Gets the value of a floating-point option.
     * @param names Option names (e.g., "r", "rate" or "r,rate").
     * @param description Option description.
     * @param defaultValue The default value.
     * @return The parsed floating-point value or the default value. If the provided value cannot be converted to a floating-point number, the program will report an error and exit.
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

private:
    // Stores option information for subsequent get/hasFlag calls.
    // key: Option name (e.g., "-o", "--output").
    // value: index > 0: Index of the argument in argv;
    // index < 0: Index of the flag option in argv;
    // index == 0: Default value, usually indicating --opt=val form.
    struct OptionInfo {
        int         argvIndex;
        std::string valueFromEquals; // Only used for --opt=val form
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

    struct InternalData {
        int    argc;
        char **argv;
        size_t positionalIdx;
        // Containers
        std::unordered_map<std::string, OptionInfo> options;
        std::vector<OptionHelpInfo>                 optionHelpEntries;
        std::vector<int>                            positionalArgsIndices;
        std::vector<PositionalHelpInfo>             positionalHelpEntries;
        std::vector<std::string>                    errorMessages;
    };

    // Internal data storage
    static inline std::string  programDescription_;
    static inline std::string  programName_;
    static inline size_t       descriptionIndent_ = 25; // NOLINT(readability-magic-numbers)
    static inline InternalData data_;

    // Internal helper functions
    // Get functions, internal data can be changed
    static inline bool                     hasFlag_(std::string_view optName, const std::string &description, InternalData &data = data_);
    static inline std::string              getString_(std::string_view optName, const std::string &description, const std::string &defaultValue = "", InternalData &data = data_);
    static inline long long                getInt_(std::string_view optName, const std::string &description, long long defaultValue = 0, InternalData &data = data_);
    static inline double                   getDouble_(std::string_view optName, const std::string &description, double defaultValue = 0.0, InternalData &data = data_);
    static inline bool                     getBool_(std::string_view optName, const std::string &description, bool defaultValue = false, InternalData &data = data_);
    static inline std::string              getPositional_(const std::string &name, const std::string &description, bool required = true, InternalData &data = data_);
    static inline std::vector<std::string> getRemainingPositionals_(const std::string &name, const std::string &description, bool isRequired = true, InternalData &data = data_);
    // Helper functions for get functions
    static inline void appendOptValErrorMsg(InternalData &data, std::string_view optName, const std::string &typeName, const std::string &valueStr);
    static inline void fixPositionalArgsArray(std::vector<int> &positionalArgsIndices, std::unordered_map<std::string, OptionInfo> &options);
    template <typename T>
    static inline std::string toString(const T &val);
    // Helper functions for get functions with long return types
    static inline std::string                         parseOptName(std::string_view optName);
    static inline std::pair<std::string, std::string> parseOptNameAsPair(std::string_view optName);
    static inline std::pair<bool, OptionInfo>         findOption(const std::string &shortOpt, const std::string &longOpt, InternalData &data);
    static inline std::pair<bool, std::string>        getValueStr(std::string_view optName, const std::string &description, const std::string &defaultValueStr, InternalData &data);
    // Other helper functions
    static inline void preprocess_(int argc, char **argv, InternalData &data = data_);
    static inline void tryToPrintHelp_(InternalData &data = data_);
    static inline bool tryToPrintInvalidOpts_(InternalData &data = data_, bool notExit = false);
    static inline void printHelp(const InternalData &data = data_);
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
inline void Parser::setDescription(const std::string &description) {
    programDescription_ = description;
}

inline void Parser::preprocess(int argc, char **argv) {
    preprocess_(argc, argv);
}

inline bool Parser::hasFlag(std::string_view optName, const std::string &description) {
    return hasFlag_(optName, description);
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

inline std::string Parser::getPositional(const std::string &name, const std::string &description, bool isRequired) {
    return getPositional_(name, description, isRequired);
}

inline std::vector<std::string> Parser::getRemainingPositionals(const std::string &name, const std::string &description, bool required) {
    return getRemainingPositionals_(name, description, required);
}

inline void Parser::changeDescriptionIndent(size_t indent) { descriptionIndent_ = indent; }

inline void Parser::tryToPrintHelp() { tryToPrintHelp_(); }

inline bool Parser::tryToPrintInvalidOpts(bool notExit) { return tryToPrintInvalidOpts_(data_, notExit); }

inline bool Parser::finalize(bool notExit) { return finalize_(data_.errorMessages, notExit); }

inline bool Parser::runAllPostprocess(bool notExit) { return runAllPostprocess_(data_, notExit); }

// === Private Helper Implementations ===

inline void Parser::preprocess_(int argc, char **argv, InternalData &data) { // NOLINT(readability-function-cognitive-complexity)
    data.argc = argc;
    data.argv = argv;
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
            data.positionalArgsIndices.push_back(i);
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
                key               = arg.substr(0, equalsPos);
                value             = arg.substr(equalsPos + 1);
                data.options[key] = {0, value};
            } else {
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    data.options[key] = {i + 1, ""};
                    i++; // Consume next arg as value
                } else {
                    data.options[key] = {-i, ""}; // Flag
                }
            }
        } else if (arg.rfind('-', 0) == 0) { // Short option(s)
            // Bundled flags, e.g., -abc
            if (arg.length() > 2) {
                for (size_t j = 1; j < arg.length(); ++j) {
                    std::string key = "-";
                    key += arg[j];
                    if (j == arg.length() - 1) { // Last char might have a value
                        if (i + 1 < argc && argv[i + 1][0] != '-') {
                            data.options[key] = {i + 1, ""};
                            i++;
                        } else {
                            data.options[key] = {-i, ""}; // Flag
                        }
                    } else {
                        data.options[key] = {-i, ""}; // All but the last are flags
                    }
                }
            } else { // Single short option, e.g., -a
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    data.options[arg] = {i + 1, ""};
                    i++;
                } else {
                    data.options[arg] = {-i, ""}; // Flag
                }
            }
        } else { // Positional
            data.positionalArgsIndices.push_back(i);
        }
    }
}

inline bool Parser::hasFlag_(
    std::string_view optName, const std::string &description, InternalData &data) {

    auto [shortOpt, longOpt] = parseOptNameAsPair(optName);
    data.optionHelpEntries.push_back({shortOpt, longOpt, description, ""});

    auto [found, info] = findOption(shortOpt, longOpt, data);

    if (found) {
        // A flag was passed with a value, e.g., -f 123. The value is likely a positional arg.
        if (info.argvIndex > 0) {
            data.positionalArgsIndices.push_back(info.argvIndex);
        }
        if (!shortOpt.empty()) data.options.erase(shortOpt);
        if (!longOpt.empty()) data.options.erase(longOpt);
    }

    return found;
}

inline std::string Parser::getString_(
    std::string_view optName, const std::string &description, const std::string &defaultValue,
    InternalData &data) {

    auto [found, value] = getValueStr(optName, description, defaultValue, data);
    return value;
}

inline long long Parser::getInt_(
    std::string_view optName, const std::string &description, long long defaultValue,
    InternalData &data) {

    auto [found, valueStr] = getValueStr(optName, description, toString(defaultValue), data);
    if (!found) {
        return defaultValue;
    }
    try {
        return std::stoll(valueStr);
    } catch (const std::exception &) {
        appendOptValErrorMsg(data, parseOptName(optName), "integer", valueStr);
    }
    return 0; // Should not reach here
}

inline double Parser::getDouble_(
    std::string_view optName, const std::string &description, double defaultValue,
    InternalData &data) {

    auto [found, valueStr] = getValueStr(optName, description, toString(defaultValue), data);
    if (!found) {
        return defaultValue;
    }
    try {
        return std::stod(valueStr);
    } catch (const std::exception &) {
        appendOptValErrorMsg(data, parseOptName(optName), "double", valueStr);
    }
    return 0.0; // Should not reach here
}

inline bool Parser::getBool_(
    std::string_view optName, const std::string &description, bool defaultValue,
    InternalData &data) {

    auto [found, valueStr] = getValueStr(optName, description, defaultValue ? "true" : "false", data);
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

    appendOptValErrorMsg(data, parseOptName(optName), "boolean", valueStr);
    return false; // Should not reach here
}

inline std::string Parser::getPositional_(
    const std::string &name, const std::string &description, bool isRequired,
    InternalData &data) {

    fixPositionalArgsArray(data.positionalArgsIndices, data.options);

    data.positionalHelpEntries.push_back({name, description, isRequired});
    if (data.positionalIdx < data.positionalArgsIndices.size()) {
        int argvIdx = data.positionalArgsIndices[data.positionalIdx];
        data.positionalIdx++;
        return data.argv[argvIdx];
    }
    if (isRequired) {
        data.errorMessages.push_back(std::string("Missing required positional argument '").append(name).append("'."));
    }
    return "";
}

inline std::vector<std::string> Parser::getRemainingPositionals_(
    const std::string &name, const std::string &description, bool required,
    InternalData &data) {

    fixPositionalArgsArray(data.positionalArgsIndices, data.options);

    data.positionalHelpEntries.push_back({name, description, required});
    std::vector<std::string> remaining;
    while (data.positionalIdx < data.positionalArgsIndices.size()) {
        int argvIdx = data.positionalArgsIndices[data.positionalIdx];
        remaining.emplace_back(data.argv[argvIdx]);
        data.positionalIdx++;
    }
    if (required && remaining.empty()) {
        data.errorMessages.push_back(std::string("Missing required positional argument(s) '").append(name).append("'."));
    }
    return remaining;
}

inline void Parser::appendOptValErrorMsg(
    InternalData    &data,
    std::string_view optName, const std::string &typeName, const std::string &valueStr) {

    std::string errorStr;
    errorStr += "Invalid value for option '";
    errorStr += parseOptName(optName);
    errorStr += "'. Expected a ";
    errorStr += typeName;
    errorStr += ", but got '";
    errorStr += valueStr;
    errorStr += "'.";
    data.errorMessages.push_back(std::move(errorStr));
}

template <typename T>
inline std::string Parser::toString(const T &val) {
    std::stringstream ss;
    ss << val;
    std::string result(ss.str());
    if constexpr (std::is_floating_point_v<T>) {
        if (result.find('.') == std::string::npos) {
            result.append(".0");
        }
    }
    return result;
}

// Parses option name (e.g., "o,out") and return a formatted string (e.g., "-o, --out")
inline std::string Parser::parseOptName(std::string_view optName) {
    auto [shortOpt, longOpt] = parseOptNameAsPair(optName);

    // Short option only
    if (longOpt.empty()) { return shortOpt; }

    // Long option only
    if (shortOpt.empty()) { return longOpt; }

    // Short and long options combined
    return std::string(shortOpt).append(", ").append(longOpt);
}

// Parses option name (o,out) and saves the results in shortOpt (-o) and longOpt (--out)
inline std::pair<std::string, std::string> Parser::parseOptNameAsPair(std::string_view optName) {
    if (optName.empty()) {
        std::cerr << "[ArgLite] Error: Option name in hasFlag/get* functions cannot be empty." << '\n';
        std::exit(EXIT_FAILURE);
    }

    std::string shortOpt;
    std::string longOpt;
    // Short option only
    if (optName.length() == 1) {
        shortOpt.append("-").append(optName);
    }
    // Long option only
    else if (optName.length() > 1 && optName[1] != ',') {
        longOpt.append("--").append(optName);
    }
    // Short and long options combined
    else {
        shortOpt.append("-").append(optName.substr(0, 1));
        longOpt.append("--").append(optName.substr(2));
    }

    return {shortOpt, longOpt};
}

// Finds the option in the options_ map
inline std::pair<bool, Parser::OptionInfo> Parser::findOption(
    const std::string &shortOpt, const std::string &longOpt, InternalData &data) {

    auto longIt  = longOpt.empty() ? data.options.end() : data.options.find(longOpt);
    auto shortIt = shortOpt.empty() ? data.options.end() : data.options.find(shortOpt);

    bool longFound  = longIt != data.options.end();
    bool shortFound = shortIt != data.options.end();

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

// Uses the option name to get a value string from the options_ map.
inline std::pair<bool, std::string> Parser::getValueStr(
    std::string_view optName, const std::string &description,
    const std::string &defaultValueStr, InternalData &data) {

    auto [shortOpt, longOpt] = parseOptNameAsPair(optName);
    data.optionHelpEntries.push_back({shortOpt, longOpt, description, defaultValueStr});

    auto [found, optInfo] = findOption(shortOpt, longOpt, data);

    if (found) {
        if (!shortOpt.empty()) data.options.erase(shortOpt);
        if (!longOpt.empty()) data.options.erase(longOpt);

        if (optInfo.argvIndex < 0) { // It's treated as a flag, indicating that it has no value
            data.errorMessages.push_back(std::string("Option '").append(parseOptName(optName)).append("' requires a value."));
            return {false, ""};
        }
        if (optInfo.argvIndex == 0) { // From --opt=val
            return {true, optInfo.valueFromEquals};
        }
        return {true, data.argv[optInfo.argvIndex]};
    }

    return {false, defaultValueStr};
}

inline void Parser::fixPositionalArgsArray(
    std::vector<int> &positionalArgsIndices, std::unordered_map<std::string, OptionInfo> &options) {
    for (auto &it : options) {
        if (it.second.argvIndex > 0) { // Unrecognized option that consumed a positional arg
            positionalArgsIndices.push_back(it.second.argvIndex);
            it.second.argvIndex = 0; // Remove the option from the options_ map
        }
    }

    // Keep positional args sorted by their original index to maintain order
    std::sort(positionalArgsIndices.begin(), positionalArgsIndices.end());
}

inline void Parser::tryToPrintHelp_(InternalData &data) {
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
            std::cerr << ERROR_STR << "Unrecognized option '" << pair.first << "'" << '\n';
        }
        if (!notExit) { std::exit(EXIT_FAILURE); }
        return true;
    }

    return false;
}

inline void Parser::printHelp(const InternalData &data) {
    if (!programDescription_.empty()) {
        std::cout << programDescription_ << '\n'
                  << '\n';
    }

    // Usage line
    std::cout << "Usage: " << programName_;
    if (!data.optionHelpEntries.empty()) std::cout << " [OPTIONS]";
    for (const auto &p : data.positionalHelpEntries) {
        std::cout << " " << (p.required ? "" : "[") << p.name << (p.required ? "" : "]");
    }
    std::cout << '\n';

    // Positional Arguments
    if (!data.positionalHelpEntries.empty()) {
        std::cout << "\nPositional Arguments:\n";
        size_t maxNameWidth = 0;
        for (const auto &p : data.positionalHelpEntries) {
            maxNameWidth = std::max(maxNameWidth, p.name.length());
        }
        for (const auto &p : data.positionalHelpEntries) {
            std::cout << "  " << std::left << std::setw(static_cast<int>(maxNameWidth) + 2) << p.name << p.description << '\n';
        }
    }

    // Options
    if (!data.optionHelpEntries.empty()) {
        std::cout << "\nOptions:" << '\n';

        for (const auto &o : data.optionHelpEntries) {
            std::string optStr("  ");
            if (!o.shortOpt.empty()) {
                optStr += o.shortOpt;
                if (!o.longOpt.empty()) { optStr += ", "; }
            } else {
                optStr += "    "; // Pad for alignment
            }
            optStr += o.longOpt;

            std::cout << std::left << std::setw(static_cast<int>(descriptionIndent_)) << optStr;

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

    std::cerr << "Errors occurred while parsing command-line arguments. The following is a list of error messages:\n";
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
