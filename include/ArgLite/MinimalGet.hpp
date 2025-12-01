#pragma once

#include "Minimal.hpp"
#include <sstream>
#include <string_view>
#include <type_traits>

namespace ArgLite {

inline bool Parser::hasFlag_(
    std::string_view optName, const std::string &description, InternalData &data) {

    auto [shortOpt, longOpt] = parseOptNameAsPair(optName);
    data.optionHelpEntries.push_back({shortOpt, longOpt, description, ""});

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

bool Parser::hasMutualExFlag_(const GetMutualExArgs &args, InternalData &data) {
    auto [shortTrueOpt, longTrueOpt]   = parseOptNameAsPair(args.trueOptName);
    auto [shortFalseOpt, longFalseOpt] = parseOptNameAsPair(args.falseOptName);

    data.optionHelpEntries.push_back({shortTrueOpt, longTrueOpt, args.trueDescription, "", "", args.defaultValue});
    data.optionHelpEntries.push_back({shortFalseOpt, longFalseOpt, args.falseDescription, "", "", !args.defaultValue});

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

inline std::string Parser::getString_(
    std::string_view optName, const std::string &description, const std::string &defaultValue,
    InternalData &data) {

    auto [found, value] = getValueStr(optName, description, defaultValue, "string", data);
    return value;
}

inline long long Parser::getInt_(
    std::string_view optName, const std::string &description, long long defaultValue,
    InternalData &data) {

    auto [found, valueStr] = getValueStr(optName, description, toString(defaultValue), "integer", data);
    if (!found) {
        return defaultValue;
    }
    try {
        return std::stoll(valueStr);
    } catch (const std::exception &) {
        appendOptValErrorMsg(data, optName, "integer", valueStr);
    }
    return 0;
}

inline double Parser::getDouble_(
    std::string_view optName, const std::string &description, double defaultValue,
    InternalData &data) {

    auto [found, valueStr] = getValueStr(optName, description, toString(defaultValue), "float", data);
    if (!found) {
        return defaultValue;
    }
    try {
        return std::stod(valueStr);
    } catch (const std::exception &) {
        appendOptValErrorMsg(data, optName, "float", valueStr);
    }
    return 0.0;
}

inline bool Parser::getBool_(
    std::string_view optName, const std::string &description, bool defaultValue,
    InternalData &data) {

    auto [found, valueStr] = getValueStr(optName, description, defaultValue ? "true" : "false", "bool", data);
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

    appendOptValErrorMsg(data, parseOptName(optName), "bool", valueStr);
    return false;
}

inline std::string Parser::getPositional_(
    const std::string &posName, const std::string &description, bool isRequired,
    const std::string &defaultValue, InternalData &data) {

    fixPositionalArgsArray(data.positionalArgsIndices, data.options);

    data.positionalHelpEntries.push_back({posName, description, defaultValue, isRequired});

    if (data.positionalIdx < data.positionalArgsIndices.size()) {
        int argvIdx = data.positionalArgsIndices[data.positionalIdx];
        data.positionalIdx++;
        return argv_[argvIdx];
    }

    if (isRequired) {
        appendPosValErrorMsg(data, posName, "Missing required positional argument '");
    }
    return defaultValue;
}

inline std::vector<std::string> Parser::getRemainingPositionals_(
    const std::string &posName, const std::string &description, bool required,
    const std::vector<std::string> &defaultValue, InternalData &data) {

    fixPositionalArgsArray(data.positionalArgsIndices, data.options);

    // Construct the default value string
    std::string defaultValueStr("[\"");
    for (unsigned long long i = 0; i < defaultValue.size(); i++) {
        if (i != 0) { defaultValueStr.append("\", \""); }
        defaultValueStr.append(defaultValue[i]);
    }
    defaultValueStr.append("\"]");
    if (defaultValue.empty()) { defaultValueStr.clear(); }

    data.positionalHelpEntries.push_back({posName, description, std::move(defaultValueStr), required, true});

    std::vector<std::string> remaining;
    while (data.positionalIdx < data.positionalArgsIndices.size()) {
        int argvIdx = data.positionalArgsIndices[data.positionalIdx];
        remaining.emplace_back(argv_[argvIdx]);
        data.positionalIdx++;
    }

    if (remaining.empty()) {
        if (required) {
            appendPosValErrorMsg(data, posName, "Missing required positional argument(s) '");
        } else {
            return defaultValue;
        }
    }
    return remaining;
}

inline void Parser::appendOptValErrorMsg(
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

inline void Parser::appendPosValErrorMsg(InternalData &data, std::string_view posName, std::string_view errorMsg) {
    std::string msg(errorMsg);
#ifdef ARGLITE_ENABLE_FORMATTER
    msg.append(Formatter::bold(posName));
#else
    msg.append(posName);
#endif
    msg.append("'.");
    data.errorMessages.push_back(std::move(msg));
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
inline Parser::OptMap::node_type Parser::findOption(
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

// Uses the option name to get a value string from the options_ map.
inline std::pair<bool, std::string> Parser::getValueStr(std::string_view optName, const std::string &description, const std::string &defaultValueStr, const std::string &typeName, InternalData &data) {

    auto [shortOpt, longOpt] = parseOptNameAsPair(optName);
    data.optionHelpEntries.push_back({shortOpt, longOpt, description, defaultValueStr, typeName});

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

} // namespace ArgLite
