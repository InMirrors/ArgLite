#include "argparse/argparse.hpp"

using namespace std;

struct MyArgs : public argparse::Args {
    int &counter               = kwarg("c,count", "Counter").set_default(0);
    bool &verbose              = flag("v,verbose", "Enable verbose mode");
    string &outputFile         = arg("Output file name").set_default("");
    vector<string> &inputFiles = arg("Input file names").multi_argument();

    void welcome() override {
        cout << "argparse example program" << '\n';
    }
};

int main(int argc, char* argv[]) {
    auto args = argparse::parse<MyArgs>(argc, argv);

    cout << "Verbose    : " << boolalpha << args.verbose << '\n';
    cout << "Counter    : " << args.counter << '\n';
    cout << "Output file: " << args.outputFile << '\n';
    cout << "Input files:" << '\n';
    for (const auto &it : args.inputFiles) { cout << it << '\n'; }

    return 0;
}