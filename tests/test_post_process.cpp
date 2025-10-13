#define ARGLITE_ENABLE_FORMATTER

#include "ArgLite/Core.hpp"
#include <iostream>
#include <string>

using namespace std;
using ArgLite::Parser;

int main(int argc, char **argv) {
    Parser::setDescription("A simple program to test ArgLite post-processing features.");
    Parser::preprocess(argc, argv);

    auto num              = Parser::getInt("n,number", "An integer.");
    auto exitIfInvalidOpt = Parser::hasFlag("i,exit-if-invalid-opt", "Exit if any invalid options are encountered.");
    auto exitIfError      = Parser::hasFlag("e,exit-if-error", "Exit if any errors are encountered.");
    auto exitIfAny        = Parser::hasFlag("a,exit-if-any", "Exit if any of the above conditions are met.");

    Parser::tryToPrintHelp();

    auto result_tryToPrintInvalidOpts = Parser::tryToPrintInvalidOpts(!exitIfInvalidOpt);
    auto result_finalize              = Parser::finalize(!exitIfError);
    auto result_runAllPostprocess     = Parser::runAllPostprocess(!exitIfAny);

    cout << boolalpha;
    cout << "Result of tryToPrintInvalidOpts(): " << result_tryToPrintInvalidOpts << '\n';
    cout << "Result of finalize()             : " << result_finalize << '\n';
    cout << "Result of runAllPostprocess()    : " << result_runAllPostprocess << '\n';
    cout << "Number                           : " << num << '\n';

    return 0;
}
