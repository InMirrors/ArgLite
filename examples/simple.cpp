#include "ArgLite/Minimal.hpp"

using namespace std;
using ArgLite::Parser;

int main(int argc, char **argv) {
    // Step 1: Preprocessing
    Parser::preprocess(argc, argv);

    // Step 2: Get command line arguments
    auto verbose    = Parser::hasFlag("v,verbose", "Enable verbose output.");
    auto number     = Parser::getInt("number", "Number of iterations."); // long option only
    auto rate       = Parser::getDouble("r", "Rate.", 123.0); // short option only, with default value
    auto outputPath = Parser::getString("o,out-path", "Output file Path.", ".");
    auto outputFile = Parser::getPositional("output-file", "The output file name.");
    auto inputFiles = Parser::getRemainingPositionals("input-files", "The input files to process.");

    // Step 3: Postprocessing
    Parser::changeDescriptionIndent(27);
    Parser::runAllPostprocess();

    cout << "Verbose    : " << boolalpha << verbose << '\n';
    cout << "Number     : " << number << '\n';
    cout << "Rate       : " << rate << '\n';
    cout << "Output Path: " << outputPath << '\n';
    cout << "Output file: " << outputFile << '\n';
    cout << "Input files:" << '\n';
    for (const auto &it : inputFiles) { cout << "  " << it << '\n'; }

    return 0;
}