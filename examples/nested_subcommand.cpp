#include <string>
#include <utility>
#include <vector>

#define ARGLITE_ENABLE_FORMATTER
#include "ArgLite/Core.hpp"
#include "ArgLite/Formatter.hpp"

using namespace std;
using ArgLite::Formatter;
using ArgLite::Parser;
using ArgLite::SubParser;

// It acts like another main().
int remoteCommand(int argc, const char *const *argv) {
    Parser::setDescription("Manages a set of tracked repositories.");

    SubParser add("add", "Adds a remote named <name> for the repository at <url>.");
    SubParser remove("remove", "Remove the remote named <name>.");

    // ArgLite will treat `remote` as the current main command.
    Parser::preprocess(argc, argv);

    // The subsequent process is the same.
    auto verbose = Parser::hasFlag("v,verbose", "be verbose; must be placed before a subcommand");
    auto name    = Parser::get<string>("-n,name", "Current repo name");
    auto posVec  = Parser::getRemainingPositionals("pos", "Optional positional arguments.");

    auto addName = add.getPositional("name", "The name of the remote to add.", true);
    auto addUrl  = add.getPositional("url", "The URL of the remote to add.", true);

    auto removeName = remove.getPositional("name", "The name of the remote to remove.", true);

    Parser::runAllPostprocess();

    if (Parser::isMainCmdActive()) {
        cout << Formatter::bold("Remote") << " command is active." << '\n';
        cout << "Verbose: " << verbose << '\n';
        cout << "Positional arguments:\n";
        for (const auto &pos : posVec) { cout << "  " << pos << '\n'; }
    }

    if (add.isActive()) {
        cout << Formatter::bold("Remote Add") << " command is active." << '\n';
        cout << "name: " << addName << '\n';
        cout << "url: " << addUrl << '\n';
    }

    if (remove.isActive()) {
        cout << Formatter::bold("Remote Remove") << " command is active." << '\n';
        cout << "name: " << removeName << '\n';
    }

    return 0;
}

// Helper function to reconstruct command-line arguments.
// It transforms `program remote [args...]` into `"program remote", [args...]`.
pair<int, const vector<const char *>> getNewCliArgs(
    int argc, const char *const *argv, string &commandName) {

    int newArgc = argc - 1;

    // Merge the program name and subcommand name into a single string,
    // which will become the new argv[0]. This is to ensure ArgLite
    // displays the help message correctly.
    // e.g., `Usage: program remote [SUBCOMMAND] [OPTIONS]`
    commandName = argv[0];
    commandName.append(" ").append(argv[1]);

    // Construct the new argv vector.
    vector<const char *> argvVec;
    argvVec.reserve(newArgc);
    argvVec.push_back(commandName.c_str());

    // Copy the rest of the arguments
    for (int i = 2; i < argc; ++i) {
        argvVec.push_back(argv[i]);
    }

    return {newArgc, argvVec};
}

int main(int argc, char **argv) {
    // Key step: Manually dispatch before any ArgLite interfaces are called.
    if (argc > 1 && string(argv[1]) == "remote") {
        // We need a string to hold the new command name.
        string commandName;
        auto [newArgc, newArgv] = getNewCliArgs(argc, argv, commandName);
        // Handle the `remote` subcommand and its subcommands.
        return remoteCommand(newArgc, newArgv.data());
    }

    // === Main command's argument parsing logic begins here ===

    // Set program infomation
    Parser::setDescription("A simple program to demonstrate the ArgLite subcommand feature.");
    Parser::setVersion("1.2.3");

    Parser::setShortNonFlagOptsStr("i");

    // Add subcommands
    SubParser status("status", "Show the working tree status");
    SubParser commit("commit", "Record changes to the repository");
    SubParser remote("remote", "Manages a set of tracked repositories.");

    commit.setShortNonFlagOptsStr("mF");

    // Preprocess
    Parser::preprocess(argc, argv);

    // Get the arguments of the main command
    auto verbose    = Parser::countFlag("v,verbose", "Enable verbose output.");
    auto enableX    = Parser::hasMutualExFlag({"x,enable-x", "Enable feature x.", "X,disable-x", "Disable feature x.", false});
    auto indent     = Parser::get<int>("i,indent", "Option Description indent.").setDefault(26).setTypeName("num").get();
    auto outputFile = Parser::getPositional("output-file", "The output file name.");
    auto inputFiles = Parser::getRemainingPositionals("input-files", "The input files to process.");

    // Get the arguments of the subcommand commit
    auto commitAll      = commit.hasFlag("a,all", "Commit all changes.");
    auto commitSquash   = commit.hasFlag("squash", "Squash all changes into one commit.");
    auto commitSignOff  = commit.hasMutualExFlag({
        "s,signoff",
        "Add a Signed-off-by trailer by the committer at the\n"
         "end of the commit log message.",
        "no-signoff",
        "Do not add a Signed-off-by trailer by the committer\n"
         "at the end of the commit log message.",
        false,
    });
    auto commitMsg      = commit.get<string>("m,message", "Use the given <msg> as the commit message.").required().get();
    auto commitFile     = commit.get<string>("F,file", "Take the commit message from the given file.").get();
    auto commitDate     = commit.get<int>("date", "Override the author date used in the commit.").get();
    auto commitPathSpec = commit.getRemainingPositionals("pathspec", " When pathspec is given on the command line, ...", false);

    // Set the help footer
    string footer;
    footer += Formatter::boldUnderline("Examples:\n");
    footer += "  subcommand -v out.txt in1.txt in2.txt\n";
    footer += "  subcommand status\n";
    footer += "  subcommand commit -m \"An awesome commit\"";
    Parser::setHelpFooter(footer);

    // Postprocess
    Parser::changeDescriptionIndent(indent);
    Parser::runAllPostprocess();

    cout << boolalpha;

    if (Parser::isMainCmdActive()) {
        cout << "Verbose    : " << verbose << '\n';
        cout << "Feature X  : " << enableX << '\n';
        cout << "Indent     : " << indent << '\n';
        cout << "Output file: " << outputFile << '\n';
        cout << "Input files:\n";
        for (const auto &it : inputFiles) { cout << "  " << it << '\n'; }
    }

    if (status.isActive()) {
        cout << Formatter::bold("Status") << " command is active." << '\n';
    }

    if (commit.isActive()) {
        cout << Formatter::bold("Commit") << " command is active." << '\n';
        cout << "all     : " << commitAll << '\n';
        cout << "squash  : " << commitSquash << '\n';
        cout << "signoff : " << commitSignOff << '\n';
        cout << "message : " << commitMsg << '\n';
        cout << "file    : " << commitFile << '\n';
        cout << "date    : " << commitDate << '\n';
        cout << "pathspec:\n";
        for (const auto &it : commitPathSpec) { cout << "  " << it << '\n'; }
    }

    return 0;
}
