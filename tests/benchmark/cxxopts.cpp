#include "cxxopts.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char **argv) {
    cxxopts::Options options("cxxopts_example", "cxxopts example program");

    options.add_options()
        ("v,verbose", "Enable verbose mode", cxxopts::value<bool>()->default_value("false"))
        ("c,count", "Counter", cxxopts::value<int>()->default_value("0"))
        ("output-file", "Output file name", cxxopts::value<string>()->default_value(""))
        ("input-files", "Input file names", cxxopts::value<vector<string>>()->default_value(""))
        ("h,help", "Print usage")
    ;

    options.parse_positional({"output-file", "input-files"});

    cxxopts::ParseResult result;
    try {
        result = options.parse(argc, argv);
    } catch (const cxxopts::exceptions::exception &e) {
        cerr << "Error parsing options: " << e.what() << '\n';
        cerr << options.help() << '\n';
        return 1;
    }

    if (result.count("help") != 0)
    {
      std::cout << options.help() << '\n';
      exit(0);
    }

    string outputFile;
    if (result.count("output-file") != 0) {
        outputFile = result["output-file"].as<string>();
    }

    vector<string> inputFiles;
    if (result.count("input-files") != 0) {
        inputFiles = result["input-files"].as<vector<string>>();
    }

    cout << "Verbose    : " << boolalpha << result["verbose"].as<bool>() << '\n';
    cout << "Counter    : " << result["count"].as<int>() << '\n';
    cout << "Output file: " << outputFile << '\n';
    cout << "Input files:\n";
    for (const auto &it : inputFiles) {
        cout << it << '\n';
    }

    return 0;
}
