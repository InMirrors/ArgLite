#define ARGLITE_ENABLE_FORMATTER

#include "ArgLite/Minimal.hpp"

using namespace std;
using ArgLite::Parser;

int main(int argc, char **argv) {
    Parser::setDescription("A simple program to demonstrate ArgLite option grouping.");
    Parser::setVersion("1.2.3");
    Parser::setShortNonFlagOptsStr("efm");
    Parser::preprocess(argc, argv);

    // insertOptHeader() removes the default "Options:" header
    // You need to add your custom header here
    Parser::insertOptHeader("Input Options");
    auto regexp = Parser::getString("e,regexp", "A pattern to search for.");
    auto file   = Parser::getString("f,file", "Search for patterns from the given file.");

    Parser::insertOptHeader("Search Options");
    auto ignoreCase = Parser::hasMutualExFlag({
        "i,ignore-case",
        "Case insensitive search.",
        "s,case-sensitive",
        "Search case sensitively",
        false,
    });
    auto maxCount   = Parser::getInt("m,max-count", "Limit the number of matching lines.");

    // Add a header for "-h, --help" and "-V, --version"
    Parser::insertOptHeader("Other Behaviors");

    Parser::runAllPostprocess();

    cout << "Regexp     : " << regexp << '\n';
    cout << "File       : " << file << '\n';
    cout << "Ignore Case: " << boolalpha << ignoreCase << '\n';
    cout << "Max Count  : " << maxCount << '\n';

    return 0;
}
