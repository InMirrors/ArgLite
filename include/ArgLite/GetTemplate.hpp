#pragma once

#include "Core.hpp"
#include <type_traits>

namespace ArgLite {

template <> inline std::string        Parser::convertType(const std::string &valueStr) { return valueStr; }
template <> inline int                Parser::convertType(const std::string &valueStr) { return std::stoi(valueStr); }
template <> inline long               Parser::convertType(const std::string &valueStr) { return std::stol(valueStr); }
template <> inline long long          Parser::convertType(const std::string &valueStr) { return std::stoll(valueStr); }
template <> inline unsigned int       Parser::convertType(const std::string &valueStr) { return std::stoul(valueStr); }
template <> inline unsigned long      Parser::convertType(const std::string &valueStr) { return std::stoul(valueStr); }
template <> inline unsigned long long Parser::convertType(const std::string &valueStr) { return std::stoull(valueStr); }
template <> inline float              Parser::convertType(const std::string &valueStr) { return std::stof(valueStr); }
template <> inline double             Parser::convertType(const std::string &valueStr) { return std::stod(valueStr); }
template <> inline bool               Parser::convertType(const std::string &valueStr) {
    auto valStrCopy = valueStr;
    std::transform(valStrCopy.begin(), valStrCopy.end(), valStrCopy.begin(), ::tolower);
    if (valStrCopy == "true" || valStrCopy == "1" || valStrCopy == "yes" || valStrCopy == "on") {
        return true;
    }
    if (valStrCopy == "false" || valStrCopy == "0" || valStrCopy == "no" || valStrCopy == "off") {
        return false;
    }
    throw std::invalid_argument("Invalid boolean value: " + valueStr);
    return false; // Should not reach here
}

template <typename T> inline T Parser::convertType(const std::string &valueStr) { // remaining types
    if constexpr (isOptionalType<T>::value) {
        return convertType<typename T::value_type>(valueStr);
    } else {
        return T(valueStr);
    }
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

template <typename T>
std::string Parser::getTypeName() {
    // Remove const, volatile qualifiers and references for consistent type comparison
    using DecayedT = remove_cvref_t<T>;

    // Group all signed integer types as "int"
    if constexpr (
        std::disjunction_v<
            std::is_same<DecayedT, int>,
            std::is_same<DecayedT, short>,
            std::is_same<DecayedT, long>,
            std::is_same<DecayedT, long long>>) {
        return "integer";
    }
    // Group all unsigned integer types as "unsigned int"
    else if constexpr (
        std::disjunction_v<
            std::is_same<DecayedT, unsigned int>,
            std::is_same<DecayedT, unsigned short>,
            std::is_same<DecayedT, unsigned long>,
            std::is_same<DecayedT, unsigned long long>>) {
        return "unsigned int";
    }
    // Group all floating-point types as "float"
    else if constexpr (
        std::disjunction_v<
            std::is_same<DecayedT, float>,
            std::is_same<DecayedT, double>,
            std::is_same<DecayedT, long double>>) {
        return "float";
    }
    // Group all character types as "char"
    else if constexpr (
        std::disjunction_v<
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
