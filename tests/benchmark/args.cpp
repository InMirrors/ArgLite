#include "args.hxx"
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char** argv) {
    args::ArgumentParser parser("args example program");

    args::Flag verbose(parser, "verbose", "Enable verbose mode", {'v', "verbose"});
    args::ValueFlag<int> count(parser, "count", "Counter", {'c', "count"});

    args::Positional<string> outputFile(parser, "output-file", "Output file name");
    args::PositionalList<string> inputFiles(parser, "input-files", "Input file names");

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help &) {
        cout << parser;
        return 0;
    } catch (args::ParseError &e) {
        cerr << e.what() << "\n" << parser;
        return 1;
    }

    cout << "Verbose    : " << boolalpha << verbose.Get() << "\n";
    cout << "Counter    : " << (count ? args::get(count) : 0) << "\n";
    cout << "Output file: " << (outputFile ? args::get(outputFile) : "") << "\n";
    cout << "Input files:" << "\n";

    if (inputFiles) {
        for (const auto& f : args::get(inputFiles)) {
            cout << f << "\n";
        }
    }

    return 0;
}
