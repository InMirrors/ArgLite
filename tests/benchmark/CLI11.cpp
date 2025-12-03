#include "CLI/CLI.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char **argv) {
    CLI::App app("CLI11 example program");

    bool verbose{false};
    app.add_flag("-v,--verbose", verbose, "Enable verbose mode");

    int count{0};
    app.add_option("-c,--count", count, "Counter");

    string outputFile;
    app.add_option("output-file", outputFile, "Output file name");

    vector<string> inputFiles;
    app.add_option("input-files", inputFiles, "Input file names");

    CLI11_PARSE(app, argc, argv);

    cout << "Verbose    : " << boolalpha << verbose << '\n';
    cout << "Counter    : " << count << '\n';
    cout << "Output file: " << outputFile << '\n';
    cout << "Input files:" << '\n';
    for (const auto &it : inputFiles) { cout << it << '\n'; }

    return 0;
}
