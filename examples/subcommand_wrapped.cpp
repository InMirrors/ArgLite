#define ARGLITE_ENABLE_FORMATTER

#include "ArgLite/Core.hpp"
#include "ArgLite/Formatter.hpp"
#include <vector>

using namespace std;
using ArgLite::Formatter;
using ArgLite::Parser;
using ArgLite::SubParser;

struct SubCommands {
    SubParser status{"status", "Show the working tree status"};
    SubParser commit{"commit", "Record changes to the repository"};
    SubParser grep{"grep", "Print lines matching a pattern"};
    SubParser mv{"mv", "Move or rename a file, a directory, or a symlink"};

    static const SubCommands &get() {
        static SubCommands instance;
        return instance;
    }

    static void init() { (void)get(); }

    SubCommands(const SubCommands &)            = delete;
    SubCommands &operator=(const SubCommands &) = delete;

private:
    SubCommands() {
        // Handle short options with values (optional)
        Parser::setShortNonFlagOptsStr("i");
        commit.setShortNonFlagOptsStr("mF");
        grep.setShortNonFlagOptsStr("e");
    };
};

struct AppConfig {
    // ===== Subcommand Configs =====
    struct StatusConfig {
    private:
        // Only AppConfig can access the private constructor
        friend struct AppConfig;
        StatusConfig() = default;
    }; // struct StatusConfig

    struct CommitConfig {
        bool all     = commitCmd().hasFlag("a,all", "Commit all changes.");
        bool squash  = commitCmd().hasFlag("squash", "Squash all changes into one commit.");
        bool signOff = commitCmd().hasMutualExFlag({
            "s,signoff",
            "Add a Signed-off-by trailer by the committer at the\n"
            "end of the commit log message.",
            "no-signoff",
            "Do not add a Signed-off-by trailer by the committer\n"
            "at the end of the commit log message.",
            false,
        });

        string         message  = commitCmd().get<string>("m,message", "Use the given <msg> as the commit message.").required().get();
        string         file     = commitCmd().get<string>("F,file", "Take the commit message from the given file.").get();
        int            date     = commitCmd().get<int>("date", "Override the author date used in the commit.").get();
        vector<string> pathSpec = commitCmd().getRemainingPositionals("pathspec", " When pathspec is given on the command line, ...", false);

    private:
        friend struct AppConfig;
        CommitConfig() = default;
    }; // struct CommitConfig

    struct GrepConfig {
        vector<string> patterns =
            grepCmd().get<string>("e,regexp", "The pattern to search for. Multiple patterns are\n"
                                              "combined by or.")
                .setTypeName("pattern")
                .getVec();

        string color =
            grepCmd().get<string>("color", "When to use colors. [possible values: auto, always,\n"
                                           "never].")
                .setDefault("auto")
                .setTypeName("when")
                .get();

    private:
        friend struct AppConfig;
        GrepConfig() {
            // Validation
            if (color != "auto" && color != "always" && color != "never") {
                string errorMsg("Invalid value for option '");
                errorMsg += Formatter::bold("--color", cerr);
                errorMsg += "'. Expected 'auto', always' or 'never', but got '";
                errorMsg += Formatter::yellow(color, cerr);
                errorMsg += "'.";
                grepCmd().pushBackErrorMsg(errorMsg);
            }
        }
    }; // struct GrepConfig

    struct MvConfig {
        bool   force = mvCmd().hasFlag("f,force", "Force renaming or moving of a file even if the target exists.");
        string src   = mvCmd().getPositional("source", "The source file or directory.");
        string dst   = mvCmd().getPositional("destination", "The destination file or directory.");

    private:
        friend struct AppConfig;
        MvConfig() = default;
    }; // struct MvConfig

    // ===== Main Command Config =====
    struct MainConfig {
        unsigned verbose = Parser::countFlag("v,verbose", "Enable verbose output.");
        bool     enableX = Parser::hasMutualExFlag({"x,enable-x", "Enable feature x.", "X,disable-x", "Disable feature x.", false});

        int            indent     = Parser::get<int>("i,indent", "Option Description indent.").setDefault(26).setTypeName("num").get();
        string         outputFile = Parser::getPositional("output-file", "The output file name.");
        vector<string> inputFiles = Parser::getRemainingPositionals("input-files", "The input files to process.");

    private:
        friend struct AppConfig;
        MainConfig() = default;
    }; // struct MainConfig

    // Command objects
    MainConfig   main;
    StatusConfig status;
    CommitConfig commit;
    GrepConfig   grep;
    MvConfig     mv;

    // It must be called after Parser::preprocess()
    static const AppConfig &get() {
        static AppConfig instance;
        return instance;
    }

    AppConfig(const AppConfig &)            = delete;
    AppConfig &operator=(const AppConfig &) = delete;

private:
    AppConfig() = default;
    // Add wrapper functions to simplify the code (Optional)
    static const SubParser &commitCmd() { return SubCommands::get().commit; }
    static const SubParser &grepCmd() { return SubCommands::get().grep; }
    static const SubParser &mvCmd() { return SubCommands::get().mv; }
}; // struct AppConfig

const AppConfig &getConfig(int argc, char **argv) {
    // Set program infomation
    Parser::setDescription("A simple program to demonstrate the ArgLite subcommand feature.");
    Parser::setVersion("1.2.3");

    // Register subcommands
    SubCommands::init();

    // Preprocess
    Parser::preprocess(argc, argv);

    // Get the config object
    const auto &config = AppConfig::get();

    // Set the help footer
    string footer;
    footer += Formatter::boldUnderline("Examples:\n");
    footer += "  subcommand -v out.txt in1.txt in2.txt\n";
    footer += "  subcommand status\n";
    footer += "  subcommand commit -m \"An awesome commit\"";
    Parser::setHelpFooter(footer);

    // Postprocess
    Parser::changeDescriptionIndent(config.main.indent);
    Parser::runAllPostprocess();

    return config;
}

int mainCmd(const AppConfig::MainConfig &config) {
    cout << "Verbose    : " << config.verbose << '\n';
    cout << "Feature X  : " << config.enableX << '\n';
    cout << "Indent     : " << config.indent << '\n';
    cout << "Output file: " << config.outputFile << '\n';
    cout << "Input files:" << '\n';
    for (const auto &it : config.inputFiles) { cout << "  " << it << '\n'; }
    return 0;
}

int statusCmd(const AppConfig::StatusConfig &config) {
    cout << ArgLite::Formatter::bold("Status") << " command is active." << '\n';
    return 0;
}

int commitCmd(const AppConfig::CommitConfig &config) {
    cout << ArgLite::Formatter::bold("Commit") << " command is active." << '\n';
    cout << "all     : " << config.all << '\n';
    cout << "squash  : " << config.squash << '\n';
    cout << "signoff : " << config.signOff << '\n';
    cout << "message : " << config.message << '\n';
    cout << "file    : " << config.file << '\n';
    cout << "date    : " << config.date << '\n';
    cout << "pathspec:" << '\n';
    for (const auto &it : config.pathSpec) { cout << "  " << it << '\n'; }
    return 0;
}

int grepCmd(const AppConfig::GrepConfig &config) {
    cout << ArgLite::Formatter::bold("Grep") << " command is active." << '\n';
    cout << "color: " << config.color << '\n';
    cout << "patterns:" << '\n';
    for (const auto &it : config.patterns) { cout << "  " << it << '\n'; }
    return 0;
}

int mvCmd(const AppConfig::MvConfig &config) {
    cout << ArgLite::Formatter::bold("Move") << " command is active." << '\n';
    cout << "force      : " << config.force << '\n';
    cout << "source     : " << config.src << '\n';
    cout << "destination: " << config.dst << '\n';
    return 0;
}

int main(int argc, char **argv) {
    const auto &subcmds = SubCommands::get();
    const auto &config  = getConfig(argc, argv);

    cout << boolalpha;

    if (Parser::isMainCmdActive()) { return mainCmd(config.main); }
    if (subcmds.status.isActive()) { return statusCmd(config.status); }
    if (subcmds.commit.isActive()) { return commitCmd(config.commit); }
    if (subcmds.grep.isActive()) { return grepCmd(config.grep); }
    if (subcmds.mv.isActive()) { return mvCmd(config.mv); }

    return 0;
}
