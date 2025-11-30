#include <iostream>
#define ARGLITE_ENABLE_FORMATTER
#ifdef MINIMAL
#define VERSION "Minimal"
#include "ArgLite/Minimal.hpp"
#else
#define VERSION "Full"
#include "ArgLite/Core.hpp"
#endif

#include "AnsiFormatter.hpp"

using namespace std;
using ArgLite::Parser;

int main(int argc, char **argv) {
    AnsiFormatter aerr(cerr);
    using C = AnsiFormatter::Color;
    aerr << "Testing " << C::BrtBlue << VERSION << C::Reset << " version" << '\n'
         << '\n';

    Parser::setDescription("Test optional positional arguments.");

    Parser::preprocess(argc, argv);

    auto outputFile = Parser::getPositional("output-file", "The output file name.");
    auto outputPath = Parser::getPositional("output-path", "The output directory name.", false, ".");
    auto inputFiles = Parser::getRemainingPositionals("input-files", "The input files to process.", false, {"input1", "input2"});

    Parser::runAllPostprocess();

    aerr << "Output file: " << outputFile << '\n';
    aerr << "Output path: " << outputPath << '\n';
    aerr << "Input files:" << '\n';
    for (const auto &it : inputFiles) { aerr << it << '\n'; }

    return 0;
}
