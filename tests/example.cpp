#define ARGLITE_ENABLE_FORMATTER

#include "ArgLite/Core.hpp"
#include <ios>
#include <iostream>
#include <string>

using namespace std;
using ArgLite::Parser;

int main(int argc, char **argv) {
    Parser::setDescription("A simple program to demonstrate ArgLite.");
    Parser::setShortNonFlagOptsStr("nr");
    Parser::preprocess(argc, argv);

    auto verbose    = Parser::hasFlag("v,verbose", "Enable verbose output.");
    auto switch1    = Parser::hasFlag("1,switch1", "Switch 1.");
    auto switch2    = Parser::hasFlag("2,switch2", "Switch 2.");
    auto debug      = Parser::getBool("d,whether-enable-debug-mode", "Whether enable debug mode.");
    auto count      = Parser::getInt("n,count", "Number of iterations.");
    auto indent     = Parser::getInt("indent", "Option Description indent.", 20); // long option only
    auto rate       = Parser::getDouble("r", "Speed rate.", 123.0);               // short option only, with default value
    auto outputPath = Parser::getString("o,out-path", "Output file Path.", "output.txt");
    auto outputFile = Parser::getPositional("output-file", "The output file name.");
    auto inputFiles = Parser::getRemainingPositionals("input-files", "The input files to process.");

    Parser::changeDescriptionIndent(indent);
    Parser::tryToPrintHelp();
    Parser::tryToPrintInvalidOpts();
    Parser::finalize();

    cout << "Verbose    : " << boolalpha << verbose << '\n';
    cout << "Switch 1   : " << switch1 << '\n';
    cout << "Switch 2   : " << switch2 << '\n';
    cout << "Debug      : " << debug << '\n';
    cout << "Count      : " << count << '\n';
    cout << "Indent     : " << indent << '\n';
    cout << "Rate       : " << rate << '\n';
    cout << "Output Path: " << outputPath << '\n';
    cout << "Output file: " << outputFile << '\n';
    cout << "Input files:" << '\n';
    for (const auto &it : inputFiles) { cout << it << '\n'; }

    return 0;
}
