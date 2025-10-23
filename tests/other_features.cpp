#define ARGLITE_ENABLE_FORMATTER

#include "ArgLite/Core.hpp"
#include <iostream>
#include <string>

using namespace std;
using ArgLite::Parser;

int main(int argc, char **argv) {
    Parser::setDescription("A simple program to demonstrate some other features of ArgLite.");
    Parser::setVersion("1.2.3");
    Parser::setShortNonFlagOptsStr("iId");
    Parser::preprocess(argc, argv);

    auto verbose   = Parser::countFlag("v,verbose", "Verbose output.");
    auto enableX   = Parser::hasMutualExFlag({"x,enable-x", "Enable feature x.", "X,disable-x", "Disable feature x.", false});
    auto indent    = Parser::get<int>("i,indent", "Option Description indent.").setDefault(26).get();
    auto delimiter = Parser::get<char>("d,delimiter", "--include delimiter.").setDefault(':').get();
    auto include   = Parser::get<string>("I,include", "Include directory.").setDefault("include").getVec(delimiter);

    Parser::changeDescriptionIndent(indent);
    Parser::runAllPostprocess();

    cout << "Verbose    : " << boolalpha << verbose << '\n';
    cout << "Feature X  : " << enableX << '\n';
    cout << "Indent     : " << indent << '\n';
    cout << "Delimiter  : '" << delimiter << "'\n";
    cout << "Include:\n";
    for (const auto &it : include) { cout << it << '\n'; }

    return 0;
}
