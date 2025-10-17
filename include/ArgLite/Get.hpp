#pragma once

#include "Core.hpp"
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

    data.optionHelpEntries.push_back({shortTrueOpt, longTrueOpt, args.trueDescription, ""});
    data.optionHelpEntries.push_back({shortFalseOpt, longFalseOpt, args.falseDescription, ""});

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
    const std::string &typeName, InternalData &data) {

    auto realTypeName = typeName.empty() ? getTypeName<std::string>() : typeName;

    auto [found, value] = getValueStr(optName, description, defaultValue, realTypeName, data);
    return value;
}

inline long long Parser::getInt_(
    std::string_view optName, const std::string &description, long long defaultValue,
    const std::string &typeName, InternalData &data) {

    auto realTypeName = typeName.empty() ? getTypeName<int>() : typeName;

    auto [found, valueStr] = getValueStr(optName, description, toString(defaultValue), realTypeName, data);
    if (!found) {
        return defaultValue;
    }
    try {
        return std::stoll(valueStr);
    } catch (const std::exception &) {
        appendOptValErrorMsg(data, optName, getTypeName<int>(), valueStr);
    }
    return 0;
}

inline double Parser::getDouble_(
    std::string_view optName, const std::string &description, double defaultValue,
    const std::string &typeName, InternalData &data) {

    auto realTypeName = typeName.empty() ? getTypeName<float>() : typeName;

    auto [found, valueStr] = getValueStr(optName, description, toString(defaultValue), realTypeName, data);
    if (!found) {
        return defaultValue;
    }
    try {
        return std::stod(valueStr);
    } catch (const std::exception &) {
        appendOptValErrorMsg(data, optName, getTypeName<float>(), valueStr);
    }
    return 0.0;
}

inline bool Parser::getBool_(
    std::string_view optName, const std::string &description, bool defaultValue,
    const std::string &typeName, InternalData &data) {

    auto realTypeName = typeName.empty() ? getTypeName<bool>() : typeName;

    auto [found, valueStr] = getValueStr(optName, description, defaultValue ? "true" : "false", realTypeName, data);
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

    appendOptValErrorMsg(data, parseOptName(optName), getTypeName<bool>(), valueStr);
    return false;
}

inline std::string Parser::getPositional_(
    const std::string &posName, const std::string &description, bool isRequired,
    InternalData &data) {

    fixPositionalArgsArray(data.positionalArgsIndices, data.options);

    data.positionalHelpEntries.push_back({posName, description, isRequired});
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
    const std::string &posName, const std::string &description, bool required,
    InternalData &data) {

    fixPositionalArgsArray(data.positionalArgsIndices, data.options);

    data.positionalHelpEntries.push_back({posName, description, required});
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
inline std::pair<bool, std::string> Parser::getValueStr(
    std::string_view optName, const std::string &description,
    const std::string &defaultValueStr, const std::string &typeName,
    InternalData &data) {

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

template <typename T>
// C++11/14/17 compatible `remove_cvref_t` (`std::remove_cvref_t` is C++20)
// This alias removes const, volatile qualifiers and references from a type T
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T>
std::string Parser::getTypeName() {
    // Remove const, volatile qualifiers and references for consistent type comparison
    using DecayedT = remove_cvref_t<T>;

    // Group all signed integer types as "int"
    if constexpr (std::disjunction_v<
                      std::is_same<DecayedT, int>,
                      std::is_same<DecayedT, short>,
                      std::is_same<DecayedT, long>,
                      std::is_same<DecayedT, long long>>) {
        return "integer";
    }
    // Group all unsigned integer types as "unsigned int"
    else if constexpr (std::disjunction_v<
                           std::is_same<DecayedT, unsigned int>,
                           std::is_same<DecayedT, unsigned short>,
                           std::is_same<DecayedT, unsigned long>,
                           std::is_same<DecayedT, unsigned long long>>) {
        return "unsigned int";
    }
    // Group all floating-point types as "float"
    else if constexpr (std::disjunction_v<
                           std::is_same<DecayedT, float>,
                           std::is_same<DecayedT, double>,
                           std::is_same<DecayedT, long double>>) {
        return "float";
    }
    // Group all character types as "char"
    else if constexpr (std::disjunction_v<
                           std::is_same<DecayedT, char>,
                           std::is_same<DecayedT, signed char>,
                           std::is_same<DecayedT, unsigned char>,
                           std::is_same<DecayedT, wchar_t>,
                           std::is_same<DecayedT, char16_t>, // C++11 character types
                           std::is_same<DecayedT, char32_t>  // C++11 character types
                           >) {
        return "char";
    }
    // Specific type for boolean
    else if constexpr (std::is_same_v<DecayedT, bool>) {
        return "bool";
    }
    // Specific type for std::string
    else if constexpr (std::is_same_v<DecayedT, std::string>) {
        return "string";
    }
    // For any other types (e.g., void, nullptr_t, pointers, custom structs), return an empty string
    else {
        return "";
    }
}

} // namespace ArgLite
