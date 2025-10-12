#include "ArgLite/Base.hpp"
#include <ios>
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

    cout << boolalpha;
    cout << "Result of tryToPrintInvalidOpts(): " << Parser::tryToPrintInvalidOpts(!exitIfInvalidOpt) << '\n';
    cout << "Result of finalize()             : " << Parser::finalize(!exitIfError) << '\n';
    cout << "Result of runAllPostprocess()    : " << Parser::runAllPostprocess(!exitIfAny) << '\n';
    cout << "Number                           : " << num << '\n';

    return 0;
}
