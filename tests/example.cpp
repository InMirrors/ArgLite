#define ARGLITE_ENABLE_FORMATTER

#include "ArgLite/Core.hpp"
#include <ios>
#include <iostream>
#include <string>

using namespace std;
using ArgLite::Parser;

int main(int argc, char **argv) {
    Parser::setDescription("A simple program to demonstrate ArgLite.");
    Parser::setVersion("1.2.3");
    Parser::setShortNonFlagOptsStr("diro");
    Parser::preprocess(argc, argv);

    auto verbose    = Parser::hasFlag("v,verbose", "Enable verbose output.");
    auto switch1    = Parser::hasFlag("1,switch1", "Switch 1.");
    auto switch2    = Parser::hasFlag("2,switch2", "Switch 2.");
    auto enableX    = Parser::hasMutualExFlag({"x,enable-x", "Enable feature x.", "X,disable-x", "Disable feature x.", false});
    auto debug      = Parser::get<bool>("d,whether-enable-debug-mode", "Whether enable debug mode.").get();
    auto indent     = Parser::get<int>("i,indent", "Option Description indent.").setDefault(26).get();
    auto number     = Parser::get<int>("number", "Number of iterations.").get();       // long option only
    auto rate       = Parser::get<double>("r", "Speed rate.").setDefault(123.0).get(); // short option only
    auto outputPath = Parser::get<string>("o,out-path", "Output file Path.").setDefault(".").get();
    auto outputFile = Parser::getPositional("output-file", "The output file name.");
    auto inputFiles = Parser::getRemainingPositionals("input-files", "The input files to process.");

    Parser::changeDescriptionIndent(indent);
    Parser::runAllPostprocess();

    cout << "Verbose    : " << boolalpha << verbose << '\n';
    cout << "Switch 1   : " << switch1 << '\n';
    cout << "Switch 2   : " << switch2 << '\n';
    cout << "Feature X  : " << enableX << '\n';
    cout << "Debug      : " << debug << '\n';
    cout << "Indent     : " << indent << '\n';
    cout << "Number     : " << number << '\n';
    cout << "Rate       : " << rate << '\n';
    cout << "Output Path: " << outputPath << '\n';
    cout << "Output file: " << outputFile << '\n';
    cout << "Input files:" << '\n';
    for (const auto &it : inputFiles) { cout << "  " << it << '\n'; }

    return 0;
}
