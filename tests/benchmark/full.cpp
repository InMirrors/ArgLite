#include "ArgLite/Core.hpp"

using namespace std;
using ArgLite::Parser;

int main(int argc, char **argv) {
    Parser::setDescription("ArgLite Full example program");
    Parser::preprocess(argc, argv);

    auto verbose    = Parser::hasFlag("v,verbose", "Enable verbose mode");
    auto counter    = Parser::get<int>("c,count", "Counter").get();
    auto outputFile = Parser::getPositional("output-file", "Output file name");
    auto inputFiles = Parser::getRemainingPositionals("input-files", "Input file names");

    Parser::runAllPostprocess();

    cout << "Verbose    : " << boolalpha << verbose << '\n';
    cout << "Counter    : " << counter << '\n';
    cout << "Output file: " << outputFile << '\n';
    cout << "Input files:" << '\n';
    for (const auto &it : inputFiles) { cout << it << '\n'; }

    return 0;
}
