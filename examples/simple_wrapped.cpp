#include "ArgLite/Minimal.hpp"

using namespace std;
using ArgLite::Parser;

class Config {
    // Ensure preprocessing runs before member initialization
    struct ArgLoader {
        ArgLoader(int argc, char **argv) {
            Parser::setShortNonFlagOptsStr("nro"); // Optional
            Parser::preprocess(argc, argv);
        }
    } loader_;

public:
    // If you only need to implement basic command-line argument parsing, simply add your arguments here
    bool verbose = Parser::hasFlag("v,verbose", "Enable verbose output.");

    int    number     = Parser::getInt("n,number", "Number of iterations.");
    double rate       = Parser::getDouble("r", "Rate.", 123.0);
    string outputPath = Parser::getString("o,out-path", "Output file Path.", ".");

    string         outputFile = Parser::getPositional("output-file", "The output file name.");
    vector<string> inputFiles = Parser::getRemainingPositionals("input-files", "The input files to process.");

    // You can ignore the following
    static Config &get(int argc, char **argv) {
        static Config config(argc, argv);
        return config;
    }

private:
    Config(int argc, char **argv) : loader_(argc, argv) {
        Parser::changeDescriptionIndent(27); // Optional
        Parser::runAllPostprocess();
    }

    Config(const Config &)            = delete;
    Config &operator=(const Config &) = delete;
};

int main(int argc, char **argv) {
    // Get the config object and use it
    auto &config = Config::get(argc, argv);

    cout << "Verbose    : " << boolalpha << config.verbose << '\n';
    cout << "Number     : " << config.number << '\n';
    cout << "Rate       : " << config.rate << '\n';
    cout << "Output Path: " << config.outputPath << '\n';
    cout << "Output file: " << config.outputFile << '\n';
    cout << "Input files:" << '\n';
    for (const auto &it : config.inputFiles) { cout << "  " << it << '\n'; }

    return 0;
}
