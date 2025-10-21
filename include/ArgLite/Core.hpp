#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef ARGLITE_ENABLE_FORMATTER
#include "Formatter.hpp"
#endif

namespace ArgLite {

class SubParser;

class Parser {
    friend class SubParser;

public:
    /**
     * @brief Sets the program description, used for the first line of the help message.
     * @param description The program's description text.
     */
    static void setDescription(std::string description) { programDescription_ = std::move(description); }

    /**
     * @brief Sets the program version and add options `-V` and `--version` to print the version.
     * @param versionStr The program's version string.
     */
    static void setVersion(std::string versionStr) { programVersion_ = std::move(versionStr); }

    /**
     * @brief Sets which short options that require a value.
     * @details To pass short options with their values as a single argument
                (e.g., `-n123` for `-n 123`),
                provide the short option names as a string to this function.
                You don't have to call it, but if you do, call it before `preprocess()`.
                Note: Only include short options that *require* a value, not all short options.
     * @param shortNonFlagOptsStr A string containing all short option characters that require a value.
                                  For example, if `-n` and `-r` require values, pass `nr`.
     */
    static void setShortNonFlagOptsStr(std::string shortNonFlagOptsStr) { mainCmdShortNonFlagOptsStr_ = std::move(shortNonFlagOptsStr); }

    /**
     * @brief Preprocesses the command-line arguments. This is the first step in using this library.
     * @param argc The argc from the main function.
     * @param argv The argv from the main function.
     */
    static void preprocess(int argc, char **argv) { preprocess_(argc, argv); }

    /**
     * @brief Checks if a flag option exists.
     * @param names Option names (e.g., "v", "verbose" or "v,verbose").
     * @param description Option description, used for the help message.
     * @return Returns true if the option appears in the command line, false otherwise.
     */
    static bool hasFlag(std::string_view optName, std::string description) {
        if (!isMainCmdActive()) { return false; }
        return hasFlag_(optName, std::move(description), data_);
    }

    //  Structure for arguments of mutually exclusive flag options.
    struct HasMutualExArgs {
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
    static bool hasMutualExFlag(HasMutualExArgs args) {
        if (!isMainCmdActive()) { return false; }
        return hasMutualExFlag_(std::move(args), data_);
    }

    template <typename T>
    class OptValBuilder;

    /**
     * @brief Gets the value of an integer option.
     * @param names Option names (e.g., "n", "count" or "n,count").
     * @param description Option description.
     * @return A OptValBuilder object that can be used to parse the option value.
     */
    template <typename T>
    static OptValBuilder<T> get(std::string_view optName, std::string description) {
        return OptValBuilder<T>(optName, std::move(description), data_, nullptr);
    }

    /**
     * @brief Gets a positional argument.
     * @details Must be called after all get/hasFlag calls. Should be called in order.
     * @param name Argument name, used for the help message (e.g., "input-file").
     * @param description Argument description.
     * @param required If true and the user does not provide the argument, the program will report an error and exit.
     * @return The string value of the argument. If the argument is not required and not provided, returns an empty string.
     */
    static std::string getPositional(const std::string &posName, std::string description, bool required = true) {
        if (!isMainCmdActive()) { return ""; }
        return getPositional_(posName, std::move(description), required, data_);
    }

    /**
     * @brief Gets all remaining positional arguments.
     * @details Must be called after all getPositional calls.
     * @param name Argument name, used for the help message (e.g., "extra-files").
     * @param description Argument description.
     * @param required If true and there are no remaining arguments, the program will report an error and exit.
     * @return A string vector containing all remaining arguments.
     */
    static std::vector<std::string> getRemainingPositionals(
        const std::string &posName, std::string description, bool required = true) {
        if (!isMainCmdActive()) { return {}; }
        return getRemainingPositionals_(posName, std::move(description), required, data_);
    }

    /**
     * @brief Changes the description indent of option descriptions in the help message. Default is 25.
     * @details This function should be called before tryToPrintHelp.
     * @param indent The new description indent.
     */
    static void changeDescriptionIndent(size_t indent) { descriptionIndent_ = indent; }

    /**
     * @brief If the user provides -h or --help, prints the help message and exits the program normally.
     * @details Recommended to be called after all get/hasFlag calls, and before tryToPrintInvalidOpts.
     */
    static void tryToPrintHelp() { tryToPrintHelp(); }

    /**
     * @brief Checks and reports all unknown options that were not processed by get/hasFlag.
     * @details If unknown options exist, prints an error message and exits the program abnormally.
     *          This function should be called after all argument retrieval function calls.
     * @param notExit If true, the program will not exit after printing any invalid options. Default is false.
     * @return Returns true if there are any unknown options and notExit is false, false otherwise.
     */
    static bool tryToPrintInvalidOpts(bool notExit = false) { return tryToPrintInvalidOpts_(data_, notExit); }

    /**
     * @brief Finalizes the parser. This function should be called at the end of parsing.
     * @details This function will print error messages and exit the program if there are any and notExit is true.
     * @param notExit If true, the program will not exit after printing any error messages. Default is false.
     * @return Returns true if there are any error messages and notExit is false, false otherwise.
     */
    static bool finalize(bool notExit = false) { return finalize_(data_, notExit); }

    /**
     * @brief Runs tryToPrintHelp, tryToPrintInvalidOpts, and finalize in sequence.
     * @param notExit If true, the program will not exit after printing any invalid options
                      or any error messages. Default is false.
     * @return Returns true if there are any invalid options or any error messages and notExit is false,
               false otherwise.
     */
    static bool runAllPostprocess(bool notExit = false) { return runAllPostprocess_(data_, notExit); }

    Parser() = delete;

    // === SubParser Instance-related Methods ===

    /**
     * @brief Checks if the main command is active.
     * @return True if the main command is active, false otherwise.
     */
    static bool isMainCmdActive() { return activeSubCmd_ == nullptr; }

private:
    // === SubParser Instance-related ===

    static inline std::vector<SubParser *> subCmdPtrs_;
    static inline SubParser               *activeSubCmd_;

    // === Static-related ===

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
        std::string typeName;
    };

    struct PositionalHelpInfo {
        std::string name;
        std::string description;
        bool        required;
    };

    using OptMap = std::unordered_map<std::string, std::vector<OptionInfo>>;

    struct InternalData {
        std::string cmdName;
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
    static inline std::string  programDescription_;
    static inline std::string  programVersion_;
    static inline std::string  mainCmdShortNonFlagOptsStr_;
    static inline InternalData data_;

    // Internal helper functions
    // Get functions, internal data can be changed
    static inline bool                     hasFlag_(std::string_view optName, std::string description, InternalData &data);
    static inline bool                     hasMutualExFlag_(HasMutualExArgs args, InternalData &data);
    static inline std::string              getPositional_(const std::string &posName, std::string description, bool required, InternalData &data);
    static inline std::vector<std::string> getRemainingPositionals_(const std::string &posName, std::string description, bool isRequired, InternalData &data);
    // Helper functions for get functions
    static inline void restorePosArgsInFlags(const std::vector<OptionInfo> &optInfoArr, std::vector<int> &positionalArgsIndices);
    static inline void appendPosValErrorMsg(InternalData &data, std::string_view posName, std::string errorMsg);
    static inline void fixPositionalArgsArray(std::vector<int> &positionalArgsIndices, OptMap &options);
    // Helper functions for get functions with long return types
    static inline std::string                         parseOptName(std::string_view optName);
    static inline std::pair<std::string, std::string> parseOptNameAsPair(std::string_view optName);
    static inline OptMap::node_type                   findOption(const std::string &shortOpt, const std::string &longOpt, InternalData &data);
    // Template helper functions for get functions
    template <typename T> struct isOptionalType : public std::false_type {};
    template <typename T> struct isOptionalType<std::optional<T>> : public std::true_type {};
    template <typename T> static inline T           convertType(const std::string &valueStr);
    template <typename T> static inline std::string toString(const T &val);
    template <typename T> static inline std::string getTypeName();
    // C++11/14/17 compatible `remove_cvref_t` (`std::remove_cvref_t` is C++20)
    // This alias removes const, volatile qualifiers and references from a type T
    template <typename T>
    using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
    // Other helper functions
    static inline void preprocess_(int argc, char **argv);
    static inline void tryToPrintVersion_(InternalData &data);
    static inline void tryToPrintHelp_(InternalData &data);
    static inline bool tryToPrintInvalidOpts_(InternalData &data, bool notExit = false);
    static inline void printHelp(const InternalData &data);
    static inline void printHelpDescription(std::string_view description);
    static inline void printHelpUsage(const InternalData &data, std::string_view cmdName);
    static inline void printHelpSubCmd(const std::vector<SubParser *> &subCmdPtrs);
    static inline void printHelpPositional(const InternalData &data);
    static inline void printHelpOptions(const InternalData &data);
    static inline void clearData(InternalData &data);
    static inline bool finalize_(InternalData &data, bool notExit = false);
    static inline bool runAllPostprocess_(InternalData &data, bool notExit = false);

#ifdef ARGLITE_ENABLE_FORMATTER
    static inline const std::string ERROR_STR = Formatter::red("Error: ");
#else
    static inline const std::string ERROR_STR = "Error: ";
#endif
};

class SubParser {
    friend Parser;

public:
    SubParser(std::string subCommandName, std::string subCmdDescription)
        : subCommandName_(std::move(subCommandName)),
          subCmdDescription_(std::move(subCmdDescription)) {

        if (std::find_if(Parser::subCmdPtrs_.begin(), Parser::subCmdPtrs_.end(), [subCommandName](const SubParser *p) {
                return p->subCommandName_ == subCommandName;
            }) != Parser::subCmdPtrs_.end()) {
            std::cerr << "[ArgLite] You cannot create multiple SubParser objects with the same subcommand name.\n";
            std::cerr << "[ArgLite] This subcommand name is already used: " << subCommandName_ << "\n";
            std::exit(EXIT_FAILURE);
        }

        Parser::subCmdPtrs_.push_back(this);
    };

    /**
     * @brief Checks if this subcommand is active.
     * @return True if this subcommand is active, false otherwise.
     */
    bool isActive() { return Parser::activeSubCmd_ == this; }

    /**
     * @brief Sets which short options that require a value.
     * @details To pass short options with their values as a single argument
                (e.g., `-n123` for `-n 123`),
                provide the short option names as a string to this function.
                You don't have to call it, but if you do, call it before `preprocess()`.
                Note: Only include short options that *require* a value, not all short options.
     * @param shortNonFlagOptsStr A string containing all short option characters that require a value.
                                  For example, if `-n` and `-r` require values, pass `nr`.
     */
    void setShortNonFlagOptsStr(std::string shortNonFlagOptsStr) { subCmdShortNonFlagOptsStr_ = std::move(shortNonFlagOptsStr); }

    /**
     * @brief Checks if a flag option exists.
     * @param names Option names (e.g., "v", "verbose" or "v,verbose").
     * @param description Option description, used for the help message.
     * @return Returns true if the option appears in the command line, false otherwise.
     */
    bool hasFlag(std::string_view optName, std::string description) {
        if (!isActive()) { return false; }
        return Parser::hasFlag_(optName, std::move(description), Parser::data_);
    }

    /**
     * @brief Checks if two mutually exclusive options exist.
     * @param args Structure containing the names and descriptions of the mutually exclusive options.
     * @return True if the first option is present and the second is not, or vice versa;
               defaultValue if neither option is present.
     */
    bool hasMutualExFlag(Parser::HasMutualExArgs args) {
        if (!isActive()) { return false; }
        return Parser::hasMutualExFlag_(std::move(args), Parser::data_);
    }

    /**
     * @brief Gets the value of an integer option.
     * @param names Option names (e.g., "n", "count" or "n,count").
     * @param description Option description.
     * @return A OptValBuilder object that can be used to parse the option value.
     */
    template <typename T>
    Parser::OptValBuilder<T> get(std::string_view optName, std::string description) {
        return Parser::OptValBuilder<T>(optName, std::move(description), Parser::data_, this);
    }

    /**
     * @brief Gets a positional argument.
     * @details Must be called after all get/hasFlag calls. Should be called in order.
     * @param name Argument name, used for the help message (e.g., "input-file").
     * @param description Argument description.
     * @param required If true and the user does not provide the argument, the program will report an error and exit.
     * @return The string value of the argument. If the argument is not required and not provided, returns an empty string.
     */
    std::string getPositional(const std::string &posName, std::string description, bool required = true) {
        if (!isActive()) { return ""; }
        return Parser::getPositional_(posName, std::move(description), required, Parser::data_);
    }

    /**
     * @brief Gets all remaining positional arguments.
     * @details Must be called after all getPositional calls.
     * @param name Argument name, used for the help message (e.g., "extra-files").
     * @param description Argument description.
     * @param required If true and there are no remaining arguments, the program will report an error and exit.
     * @return A string vector containing all remaining arguments.
     */
    std::vector<std::string> getRemainingPositionals(
        const std::string &posName, std::string description, bool required = true) {
        if (!isActive()) { return {}; }
        return Parser::getRemainingPositionals_(posName, std::move(description), required, Parser::data_);
    }

private:
    std::string subCommandName_;
    std::string subCmdDescription_;
    std::string subCmdShortNonFlagOptsStr_;
};

} // namespace ArgLite

#include "Get.hpp"            // IWYU pragma: keep
#include "PrePostProcess.hpp" // IWYU pragma: keep
