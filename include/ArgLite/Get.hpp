#pragma once

#include "Core.hpp"
#include "GetTemplate.hpp" // IWYU pragma: keep
#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace ArgLite {

inline bool Parser::hasFlag_(
    std::string_view optName, std::string description, InternalData &data) {

    auto [shortOpt, longOpt] = parseOptNameAsPair(optName);
    data.optionHelpEntries.push_back({shortOpt, longOpt, std::move(description), ""});

    auto longNode  = data.options.extract(longOpt);
    auto shortNode = data.options.extract(shortOpt);

    auto emptyArr        = std::vector<OptionInfo>();
    auto longOptInfoArr  = longNode.empty() ? emptyArr : std::move(longNode.mapped());
    auto shortOptInfoArr = shortNode.empty() ? emptyArr : std::move(shortNode.mapped());

    restorePosArgsInFlags(shortOptInfoArr, data.positionalArgsIndices);
    restorePosArgsInFlags(longOptInfoArr, data.positionalArgsIndices);

    return !(longNode.empty() && shortNode.empty());
}

bool Parser::hasMutualExFlag_(HasMutualExArgs args, InternalData &data) {
    auto [shortTrueOpt, longTrueOpt]   = parseOptNameAsPair(args.trueOptName);
    auto [shortFalseOpt, longFalseOpt] = parseOptNameAsPair(args.falseOptName);

    data.optionHelpEntries.push_back({shortTrueOpt, longTrueOpt, std::move(args.trueDescription), ""});
    data.optionHelpEntries.push_back({shortFalseOpt, longFalseOpt, std::move(args.falseDescription), ""});

    auto trueLongNode   = data.options.extract(longTrueOpt);
    auto trueShortNode  = data.options.extract(shortTrueOpt);
    auto falseLongNode  = data.options.extract(longFalseOpt);
    auto falseShortNode = data.options.extract(shortFalseOpt);

    auto emptyArr             = std::vector<OptionInfo>();
    auto trueLongOptInfoArr   = trueLongNode.empty() ? emptyArr : std::move(trueLongNode.mapped());
    auto trueShortOptInfoArr  = trueShortNode.empty() ? emptyArr : std::move(trueShortNode.mapped());
    auto falseLongOptInfoArr  = falseLongNode.empty() ? emptyArr : std::move(falseLongNode.mapped());
    auto falseShortOptInfoArr = falseShortNode.empty() ? emptyArr : std::move(falseShortNode.mapped());

    restorePosArgsInFlags(trueLongOptInfoArr, data.positionalArgsIndices);
    restorePosArgsInFlags(trueShortOptInfoArr, data.positionalArgsIndices);
    restorePosArgsInFlags(falseLongOptInfoArr, data.positionalArgsIndices);
    restorePosArgsInFlags(falseShortOptInfoArr, data.positionalArgsIndices);

    auto trueLongIndex   = trueLongOptInfoArr.empty() ? 0 : trueLongOptInfoArr.back().argvIndex;
    auto trueShortIndex  = trueShortOptInfoArr.empty() ? 0 : trueShortOptInfoArr.back().argvIndex;
    auto falseLongIndex  = falseLongOptInfoArr.empty() ? 0 : falseLongOptInfoArr.back().argvIndex;
    auto falseShortIndex = falseShortOptInfoArr.empty() ? 0 : falseShortOptInfoArr.back().argvIndex;

    auto trueIndex  = std::min(trueShortIndex, trueLongIndex);
    auto falseIndex = std::min(falseShortIndex, falseLongIndex);

    // They are negative, so Smaller is latter
    return trueIndex < falseIndex;
}

void Parser::restorePosArgsInFlags(const std::vector<OptionInfo> &optInfoArr, std::vector<int> &positionalArgsIndices) {
    for (const auto &it : optInfoArr) {
        // A flag was passed with a value, e.g., -f 123. The value is likely a positional arg.
        if (it.argvIndex > 0) {
            positionalArgsIndices.push_back(it.argvIndex);
        }
    }
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

class Parser::OptValHelper {
    template <typename T> friend class OptValBuilder;

    static void appendOptValErrorMsg(
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

    static void appendPosValErrorMsg(
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

    static bool hasNoValOpt(const std::vector<OptionInfo> &optInfoArr,
                            std::string_view optName, std::vector<std::string> &errorMessages) {
        bool hasNoValOpt = false;

        for (const auto &it : optInfoArr) {
            if (it.argvIndex < 0) { // It's treated as a flag, indicating that it has no value
                hasNoValOpt = true;
                std::string msg("Option '");
#ifdef ARGLITE_ENABLE_FORMATTER
                msg.append(Formatter::bold(parseOptName(optName)));
#else
                msg.append(parseOptName(optName));
#endif
                msg.append("' requires a value.");
                errorMessages.push_back(std::move(msg));
            }
        }

        return hasNoValOpt;
    }

    // Uses the option name to get a value string from the options_ map.
    static std::tuple<bool, std::vector<OptionInfo>, std::vector<OptionInfo>> getLongShortOptArr(
        std::string_view optName, const std::string &shortOpt, const std::string &longOpt,
        InternalData &data) {

        auto longNode  = data.options.extract(longOpt);
        auto shortNode = data.options.extract(shortOpt);

        // Both long and short options are not found
        if (longNode.empty() && shortNode.empty()) {
            return {false, {}, {}};
        }

        // At least one option is found, so at least one array is not empty
        auto emptyArr        = std::vector<OptionInfo>();
        auto longOptInfoArr  = longNode.empty() ? emptyArr : std::move(longNode.mapped());
        auto shortOptInfoArr = shortNode.empty() ? emptyArr : std::move(shortNode.mapped());

        if (hasNoValOpt(longOptInfoArr, optName, data.errorMessages) ||
            hasNoValOpt(shortOptInfoArr, optName, data.errorMessages)) {
            return {false, {}, {}};
        }

        return {true, longOptInfoArr, shortOptInfoArr};
    }

    static std::string getValueStr(
        std::vector<OptionInfo> longOptInfoArr, std::vector<OptionInfo> shortOptInfoArr) {

        auto longIndex  = longOptInfoArr.empty() ? 0 : longOptInfoArr.back().argvIndex;
        auto shortIndex = shortOptInfoArr.empty() ? 0 : shortOptInfoArr.back().argvIndex;

        auto &optInfo = longIndex > shortIndex ? longOptInfoArr.back() : shortOptInfoArr.back();
        if (!optInfo.valueStr.empty()) { return std::move(optInfo.valueStr); }
        return argv_[optInfo.argvIndex];
    }

    static std::vector<std::string> getValueStrVec(
        std::vector<OptionInfo> longOptInfoArr, std::vector<OptionInfo> shortOptInfoArr) {

        std::vector<std::string> valueStrVec;
        valueStrVec.reserve(longOptInfoArr.size() + shortOptInfoArr.size());

        size_t longIdx  = 0;
        size_t shortIdx = 0;

        while (longIdx < longOptInfoArr.size() && shortIdx < shortOptInfoArr.size()) {
            OptionInfo *currentOptInfo = nullptr;
            if (longOptInfoArr[longIdx].argvIndex < shortOptInfoArr[shortIdx].argvIndex) {
                currentOptInfo = &longOptInfoArr[longIdx++];
            } else {
                currentOptInfo = &shortOptInfoArr[shortIdx++];
            }

            if (!currentOptInfo->valueStr.empty()) {
                valueStrVec.push_back(std::move(currentOptInfo->valueStr));
            } else {
                valueStrVec.emplace_back(argv_[currentOptInfo->argvIndex]);
            }
        }

        while (longIdx < longOptInfoArr.size()) {
            if (!longOptInfoArr[longIdx].valueStr.empty()) {
                valueStrVec.push_back(std::move(longOptInfoArr[longIdx].valueStr));
            } else {
                valueStrVec.emplace_back(argv_[longOptInfoArr[longIdx].argvIndex]);
            }
            longIdx++;
        }

        while (shortIdx < shortOptInfoArr.size()) {
            if (!shortOptInfoArr[shortIdx].valueStr.empty()) {
                valueStrVec.push_back(std::move(shortOptInfoArr[shortIdx].valueStr));
            } else {
                valueStrVec.emplace_back(argv_[shortOptInfoArr[shortIdx].argvIndex]);
            }
            shortIdx++;
        }

        return valueStrVec;
    }

    static std::vector<std::string> getSplittedStrVec(
        std::vector<std::string> &valueStrVec, char delimiter) {

        std::vector<std::string> splittedStrVec;
        splittedStrVec.reserve(valueStrVec.size());

        for (auto &valueStr : valueStrVec) {
            auto delimiterPos = valueStr.find(delimiter);
            if (delimiterPos == std::string::npos) {
                splittedStrVec.push_back(std::move(valueStr));
            }
            // The delemiter is found, split the string
            else {
                size_t currentPos = 0;
                while ((delimiterPos = valueStr.find(delimiter, currentPos)) != std::string::npos) {
                    splittedStrVec.push_back(valueStr.substr(currentPos, delimiterPos - currentPos));
                    currentPos = delimiterPos + 1;
                }
                // Add the last part
                splittedStrVec.push_back(valueStr.substr(currentPos));
            }
        }

        return splittedStrVec;
    }
};

template <typename T>
class Parser::OptValBuilder {
public:
    using Helper = OptValHelper;

    OptValBuilder(std::string_view optName, std::string description,
                  InternalData &data, SubParser *passedSubCmd)
        : optName_(optName),
          description_(std::move(description)),
          data_(data),
          passedSubCmd_(passedSubCmd) {
        typeName_ = getTypeName<T>();
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

    /**
     * @brief Retrieves the option's value.
     * @return The parsed value of the option,
               or the default value if the option is not found or its value is invalid.
     */
    T get() {
        if (passedSubCmd_ != activeSubCmd_) { return defaultValue_; }

        auto [shortOpt, longOpt] = parseOptNameAsPair(optName_);
        data_.optionHelpEntries.push_back({shortOpt, longOpt, std::move(description_), toString(defaultValue_), getTypeName<T>()});

        auto [found, longOptInfoArr, shortOptInfoArr] =
            Helper::getLongShortOptArr(optName_, shortOpt, longOpt, data_);

        if (!found) { return defaultValue_; }

        auto valueStr = Helper::getValueStr(longOptInfoArr, shortOptInfoArr);

        try {
            return convertType<T>(valueStr);
        } catch (...) {
            Helper::appendOptValErrorMsg(data_, optName_, typeName_, valueStr);
        }
        return defaultValue_;
    }

    /**
     * @brief Retrieves the option's value as a vector of type T.
     *
     * @details This function is used to retrieve the value of an option that,
     *          expects multiple arguments, which are then converted to a vector<T>.
     *          It handles the parsing and conversion of the option's value,
     *          splitting it into individual elements based on the provided delimiter.
     * @param delimiter The delimiter character used to split the option's value string into
     *                  individual elements.
     *                  If `\0` is provided (the default), the value is not split,
     *                  and the entire value string is treated as a single element.
     * @return A vector of type T containing the parsed values of the option's arguments.
     *         Returns an empty vector if the option is not found or if an error occurs
     *         during value conversion.
     */
    std::vector<T> getVec(char delimiter = '\0') {
        if (passedSubCmd_ != activeSubCmd_) { return {}; }

        auto [shortOpt, longOpt] = parseOptNameAsPair(optName_);
        data_.optionHelpEntries.push_back({shortOpt, longOpt, std::move(description_), toString(defaultValue_), getTypeName<T>()});

        auto [found, longOptInfoArr, shortOptInfoArr] =
            Helper::getLongShortOptArr(optName_, shortOpt, longOpt, data_);

        if (!found) { return {}; }

        auto valueStrVec    = Helper::getValueStrVec(longOptInfoArr, shortOptInfoArr);
        auto splittedStrVec = Helper::getSplittedStrVec(valueStrVec, delimiter);

        // Convert each string to T
        std::vector<T> resultVec;
        resultVec.reserve(splittedStrVec.size());
        for (auto &valueStr : splittedStrVec) {
            try {
                resultVec.push_back(convertType<T>(valueStr));
            } catch (...) {
                Helper::appendOptValErrorMsg(data_, optName_, typeName_, valueStr);
            }
        }

        return resultVec;
    }

private:
    std::string_view optName_;
    std::string      description_;
    InternalData    &data_;
    std::string      typeName_;
    T                defaultValue_{};
    SubParser       *passedSubCmd_{nullptr};
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

    for (auto &option : options) {
        auto &optInfoArr = option.second;
        for (auto &it : optInfoArr) {
            if (it.argvIndex > 0) { // Unrecognized option that consumed a positional arg
                positionalArgsIndices.push_back(it.argvIndex);
                it.argvIndex = 0; // Remove the option from the options_ map
            }
        }
    }

    // Keep positional args sorted by their original index to maintain order
    std::sort(positionalArgsIndices.begin(), positionalArgsIndices.end());
}

} // namespace ArgLite
