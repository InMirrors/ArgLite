#define ARGLITE_ENABLE_FORMATTER
#include "ArgLite/Formatter.hpp"
#include "ArgLite/Core.hpp"

using namespace std;
using ArgLite::Parser;
using ArgLite::SubParser;

int main(int argc, char **argv) {
    // Set program infomation
    Parser::setDescription("A simple program to demonstrate the ArgLite subcommand feature.");
    Parser::setVersion("1.2.3");

    Parser::setShortNonFlagOptsStr("i");

    // Add subcommands
    SubParser status("status", "Show the working tree status");
    SubParser commit("commit", "Record changes to the repository");
    SubParser grep("grep", "Print lines matching a pattern");
    SubParser mv("mv", "Move or rename a file, a directory, or a symlink");

    commit.setShortNonFlagOptsStr("mF");
    grep.setShortNonFlagOptsStr("e");

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

    // Get the arguments of the subcommand grep
    auto grepPatterns = grep.get<string>("e,regexp", "The pattern to search for. Multiple patterns are\n"
                                                     "combined by or.")
                            .setTypeName("pattern")
                            .getVec();
    auto grepColor = grep.get<string>("color", "When to use colors. [possible values: auto, always,\n"
                                               "never].")
                         .setDefault("auto")
                         .setTypeName("when")
                         .get();
    // Validate grep color option value.
    if (grepColor != "auto" && grepColor != "always" && grepColor != "never") {
        string errorMsg("Invalid value for option '");
        errorMsg += ArgLite::Formatter::bold("--color", cerr);
        errorMsg += "'. Expected 'auto', always' or 'never', but got '";
        errorMsg += ArgLite::Formatter::yellow(grepColor, cerr);
        errorMsg += "'.";
        grep.pushBackErrorMsg(errorMsg);
    }

    // Get the arguments of the subcommand mv
    auto mvSrc   = mv.getPositional("source", "The source file or directory.");
    auto mvDst   = mv.getPositional("destination", "The destination file or directory.");
    auto mvForce = mv.hasFlag("f,force", "Force renaming or moving of a file even if the target exists.");

    // Set the help footer
    string footer;
    footer += ArgLite::Formatter::boldUnderline("Examples:\n");
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
        cout << "Input files:" << '\n';
        for (const auto &it : inputFiles) { cout << "  " << it << '\n'; }
    }

    if (status.isActive()) {
        cout << ArgLite::Formatter::bold("Status") << " command is active." << '\n';
    }

    if (commit.isActive()) {
        cout << ArgLite::Formatter::bold("Commit") << " command is active." << '\n';
        cout << "all     : " << commitAll << '\n';
        cout << "squash  : " << commitSquash << '\n';
        cout << "signoff : " << commitSignOff << '\n';
        cout << "message : " << commitMsg << '\n';
        cout << "file    : " << commitFile << '\n';
        cout << "date    : " << commitDate << '\n';
        cout << "pathspec:" << '\n';
        for (const auto &it : commitPathSpec) { cout << "  " << it << '\n'; }
    }

    if (grep.isActive()) {
        cout << ArgLite::Formatter::bold("Grep") << " command is active." << '\n';
        cout << "color: " << grepColor << '\n';
        cout << "patterns:" << '\n';
        for (const auto &it : grepPatterns) { cout << "  " << it << '\n'; }
    }

    if (mv.isActive()) {
        cout << ArgLite::Formatter::bold("Move") << " command is active." << '\n';
        cout << "force      : " << mvForce << '\n';
        cout << "source     : " << mvSrc << '\n';
        cout << "destination: " << mvDst << '\n';
    }

    return 0;
}
