#include "ArgLite/Base.hpp"
#include <ios>
#include <iostream>
#include <string>

using namespace std;
using ArgLite::Parser;

int main(int argc, char **argv) {
    Parser::setDescription("A simple program to demonstrate ArgLite.");
    Parser::preprocess(argc, argv);

    auto verbose    = Parser::hasFlag("v,verbose", "Enable verbose output.");
    auto count      = Parser::getInt("n,count", "Number of iterations.", 10);
    auto rate       = Parser::getDouble("r", "Speed rate.");
    auto outputPath = Parser::getString("o,out-path", "Output file Path.", "output.txt");
    auto debug      = Parser::getBool("d,whether-enable-debug-mode", "Whether enable debug mode.");
    auto outputFile = Parser::getPositional("output-file", "The output file name.");
    auto inputFiles = Parser::getRemainingPositionals("input-files", "The input files to process.");

    Parser::changeDescriptionIndent(20);
    Parser::tryToPrintHelp();
    Parser::tryToPrintInvalidOpts();

    cout << "Verbose    : " << boolalpha << verbose << '\n';
    cout << "Debug      : " << debug << '\n';
    cout << "Count      : " << count << '\n';
    cout << "Rate       : " << rate << '\n';
    cout << "Output Path: " << outputPath << '\n';
    cout << "Output file: " << outputFile << '\n';
    cout << "Input files:" << '\n';
    for (const auto &it : inputFiles) { cout << it << '\n'; }

    return 0;
}
