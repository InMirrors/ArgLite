#pragma once

#include "Core.hpp"
#include "GetTemplate.hpp" // IWYU pragma: keep
#include <string_view>

namespace ArgLite {

inline bool Parser::hasFlag_(
    std::string_view optName, std::string description, InternalData &data) {

    auto [shortOpt, longOpt] = parseOptNameAsPair(optName);
    data.optionHelpEntries.push_back({shortOpt, longOpt, std::move(description), ""});

    auto optNode = findOption(shortOpt, longOpt, data);

    if (!optNode.empty()) {
        const auto &optInfo = optNode.mapped();
        // A flag was passed with a value, e.g., -f 123. The value is likely a positional arg.
        if (optInfo.argvIndex > 0) {
            data.positionalArgsIndices.push_back(optInfo.argvIndex);
        }
    }

    return !optNode.empty();
}

bool Parser::hasMutualExFlag_(HasMutualExArgs args, InternalData &data) {
    auto [shortTrueOpt, longTrueOpt]   = parseOptNameAsPair(args.trueOptName);
    auto [shortFalseOpt, longFalseOpt] = parseOptNameAsPair(args.falseOptName);

    data.optionHelpEntries.push_back({shortTrueOpt, longTrueOpt, std::move(args.trueDescription), ""});
    data.optionHelpEntries.push_back({shortFalseOpt, longFalseOpt, std::move(args.falseDescription), ""});

    auto trueNode  = findOption(shortTrueOpt, longTrueOpt, data);
    auto falseNode = findOption(shortFalseOpt, longFalseOpt, data);

    if (!trueNode.empty() && trueNode.mapped().argvIndex > 0) {
        data.positionalArgsIndices.push_back(trueNode.mapped().argvIndex);
    }
    if (!falseNode.empty() && falseNode.mapped().argvIndex > 0) {
        data.positionalArgsIndices.push_back(falseNode.mapped().argvIndex);
    }

    if (trueNode.empty() && falseNode.empty()) { return args.defaultValue; }

    // They are negative, so Smaller is latter
    if (!trueNode.empty() && !falseNode.empty()) {
        return trueNode.mapped().argvIndex < falseNode.mapped().argvIndex;
    }

    if (!trueNode.empty()) { return true; }
    return false;
}

// === Helper functions for parsing option ===

// Parses option name (e.g., "o,out") and return a formatted string (e.g., "-o, --out")
std::string Parser::parseOptName(std::string_view optName) {
    auto [shortOpt, longOpt] = parseOptNameAsPair(optName);

    // Short option only
    if (longOpt.empty()) { return shortOpt; }

    // Long option only
    if (shortOpt.empty()) { return longOpt; }

    // Short and long options combined
    return std::string(shortOpt).append(", ").append(longOpt);
}

// Parses option name (o,out) and saves the results in shortOpt (-o) and longOpt (--out)
std::pair<std::string, std::string> Parser::parseOptNameAsPair(std::string_view optName) {
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
Parser::OptMap::node_type Parser::findOption(
    const std::string &shortOpt, const std::string &longOpt, InternalData &data) {

    auto longNode  = data.options.extract(longOpt);
    auto shortNode = data.options.extract(shortOpt);

    if (!longNode.empty() && !shortNode.empty()) {
        if (std::abs(longNode.mapped().argvIndex) > std::abs(shortNode.mapped().argvIndex)) {
            return longNode;
        }
        return shortNode;
    }

    if (!longNode.empty()) { return longNode; }

    return shortNode;
}

template <typename T>
class Parser::OptValBuilder {
public:
    OptValBuilder(std::string_view optName,
                  std::string description, InternalData &data)
        : optName_(optName),
          description_(std::move(description)),
          data_(data) {
        typeName_ = getTypeName<T>();
    }

    /**
     * @brief Retrieves the option's value.
     * @return The parsed value of the option,
               or the default value if the option is not found or its value is invalid.
     */
    T get() {
        auto [found, valueStr] = getValueStr(optName_, std::move(description_), toString(defaultValue_), getTypeName<T>(), data_);

        if (!found) { return defaultValue_; }

        try {
            return convertType<T>(valueStr);
        } catch (...) {
            appendOptValErrorMsg(data_, optName_, typeName_, valueStr);
        }
        return defaultValue_;
    }

    /**
     * @brief Sets the default value for the option.
     * @param defaultValue The value to be used as the default.
     * @return A reference to the current OptValBuilder instance for chaining.
     */
    OptValBuilder<T> &setDefault(T defaultValue) {
        defaultValue_ = defaultValue;
        return *this;
    }

private:
    std::string_view optName_;
    std::string      description_;
    InternalData    &data_;
    std::string      typeName_;
    T                defaultValue_{};

    void appendOptValErrorMsg(
        InternalData    &data,
        std::string_view optName, const std::string &typeName, const std::string &valueStr) {

        std::string errorStr;
        errorStr += "Invalid value for option '";
#ifdef ARGLITE_ENABLE_FORMATTER
        errorStr += Formatter::bold(parseOptName(optName));
#else
        errorStr += parseOptName(optName);
#endif
        errorStr += "'. Expected a ";
#ifdef ARGLITE_ENABLE_FORMATTER
        errorStr += Formatter::bold(typeName);
#else
        errorStr += typeName;
#endif
        errorStr += ", but got '";
#ifdef ARGLITE_ENABLE_FORMATTER
        errorStr += Formatter::bold(valueStr);
#else
        errorStr += valueStr;
#endif
        errorStr += "'.";
        data.errorMessages.push_back(std::move(errorStr));
    }

    void appendPosValErrorMsg(
        InternalData &data, std::string_view posName, std::string_view errorMsg) {

        std::string msg(errorMsg);
#ifdef ARGLITE_ENABLE_FORMATTER
        msg.append(Formatter::bold(posName));
#else
        msg.append(posName);
#endif
        msg.append("'.");
        data.errorMessages.push_back(std::move(msg));
    }

    // Parses option name (e.g., "o,out") and return a formatted string (e.g., "-o, --out")
    // Uses the option name to get a value string from the options_ map.
    std::pair<bool, std::string> getValueStr(
        std::string_view optName, std::string description,
        std::string defaultValueStr, std::string typeName,
        InternalData &data) {

        auto [shortOpt, longOpt] = parseOptNameAsPair(optName);
        data.optionHelpEntries.push_back({shortOpt, longOpt, std::move(description), std::move(defaultValueStr), std::move(typeName)});

        auto optNode = findOption(shortOpt, longOpt, data);

        if (!optNode.empty()) {
            const auto &optInfo = optNode.mapped();

            if (optInfo.argvIndex < 0) { // It's treated as a flag, indicating that it has no value
                std::string msg("Option '");
#ifdef ARGLITE_ENABLE_FORMATTER
                msg.append(Formatter::bold(parseOptName(optName)));
#else
                msg.append(parseOptName(optName));
#endif
                msg.append("' requires a value.");
                data.errorMessages.push_back(std::move(msg));
                return {false, ""};
            }
            if (!optInfo.valueStr.empty()) { // From -n123 or --opt=val form
                return {true, optInfo.valueStr};
            }
            return {true, argv_[optInfo.argvIndex]};
        }

        return {false, defaultValueStr};
    }
};

// === Positional Args ===

inline std::string Parser::getPositional_(
    const std::string &posName, std::string description, bool isRequired,
    InternalData &data) {

    fixPositionalArgsArray(data.positionalArgsIndices, data.options);

    data.positionalHelpEntries.push_back({posName, std::move(description), isRequired});
    if (positionalIdx_ < data.positionalArgsIndices.size()) {
        int argvIdx = data.positionalArgsIndices[positionalIdx_];
        positionalIdx_++;
        return argv_[argvIdx];
    }
    if (isRequired) {
        appendPosValErrorMsg(data, posName, "Missing required positional argument '");
    }
    return "";
}

inline std::vector<std::string> Parser::getRemainingPositionals_(
    const std::string &posName, std::string description, bool required,
    InternalData &data) {

    fixPositionalArgsArray(data.positionalArgsIndices, data.options);

    data.positionalHelpEntries.push_back({posName, std::move(description), required});
    std::vector<std::string> remaining;
    while (positionalIdx_ < data.positionalArgsIndices.size()) {
        int argvIdx = data.positionalArgsIndices[positionalIdx_];
        remaining.emplace_back(argv_[argvIdx]);
        positionalIdx_++;
    }
    if (required && remaining.empty()) {
        appendPosValErrorMsg(data, posName, "Missing required positional argument(s) '");
    }
    return remaining;
}

inline void Parser::appendPosValErrorMsg(
    InternalData &data, std::string_view posName, std::string errorMsg) {

    std::string msg(std::move(errorMsg));
#ifdef ARGLITE_ENABLE_FORMATTER
    msg.append(Formatter::bold(posName));
#else
    msg.append(posName);
#endif
    msg.append("'.");
    data.errorMessages.push_back(std::move(msg));
}

inline void Parser::fixPositionalArgsArray(
    std::vector<int> &positionalArgsIndices, OptMap &options) {

    for (auto &it : options) {
        if (it.second.argvIndex > 0) { // Unrecognized option that consumed a positional arg
            positionalArgsIndices.push_back(it.second.argvIndex);
            it.second.argvIndex = 0; // Remove the option from the options_ map
        }
    }

    // Keep positional args sorted by their original index to maintain order
    std::sort(positionalArgsIndices.begin(), positionalArgsIndices.end());
}

} // namespace ArgLite
