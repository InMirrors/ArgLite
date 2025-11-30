#define ARGLITE_ENABLE_FORMATTER

#include "ArgLite/Minimal.hpp"
#include <cassert>
#include <iostream>
#include <vector>

using namespace std;
using ArgLite::Parser;

// Helper function to create argv
vector<char *> create_argv(const vector<string> &args) {
    vector<char *> argv;
    for (const auto &arg : args) {
        argv.push_back(const_cast<char *>(arg.c_str()));
    }
    return argv;
}

void test_positional_only() {
    cerr << "--- Running test_positional_only ---" << '\n';
    vector<string> args = {"./getPosMinimal", "arg1", "arg2", "arg3"};
    auto             argv = create_argv(args);
    Parser::preprocess(argv.size(), argv.data());
    auto pos1 = Parser::getPositional("pos1", "First positional arg.");
    auto rem  = Parser::getRemainingPositionals("rem", "Remaining positional args.");
    Parser::runAllPostprocess();

    assert(pos1 == "arg1");
    assert(rem.size() == 2);
    assert(rem[0] == "arg2");
    assert(rem[1] == "arg3");
    cerr << "test_positional_only PASSED" << '\n'
         << '\n';
}

void test_with_flags_and_valued_options() {
    cerr << "--- Running test_with_flags_and_valued_options ---" << '\n';
    vector<string> args = {"./getPosMinimal", "-v", "--number", "123", "--file=test.txt"};
    auto             argv = create_argv(args);

    Parser::setShortNonFlagOptsStr("f"); // For valued short options if any, not used in this case but good practice
    Parser::preprocess(argv.size(), argv.data());

    auto verbose = Parser::hasFlag("v,verbose", "Enable verbose output.");
    auto number  = Parser::getInt("number", "A number.");
    auto file    = Parser::getString("f,file", "A file path.");

    Parser::runAllPostprocess();

    assert(verbose == true);
    assert(number == 123);
    assert(file == "test.txt");
    cerr << "test_with_flags_and_valued_options PASSED" << '\n'
         << '\n';
}

void test_all_together() {
    cerr << "--- Running test_all_together ---" << '\n';
    vector<string> args = {"./getPosMinimal", "pos_arg1", "-v", "--rate", "9.8", "pos_arg2", "--", "pos_arg3"};
    auto             argv = create_argv(args);

    Parser::preprocess(argv.size(), argv.data());

    auto verbose = Parser::hasFlag("v,verbose", "Enable verbose output.");
    auto rate    = Parser::getDouble("r,rate", "A rate value.");
    auto pos1    = Parser::getPositional("pos1", "First positional arg.");
    auto rem     = Parser::getRemainingPositionals("rem", "Remaining positional args.");

    Parser::runAllPostprocess();

    assert(verbose == true);
    assert(rate == 9.8);
    assert(pos1 == "pos_arg1");
    assert(rem.size() == 2);
    assert(rem[0] == "pos_arg2");
    assert(rem[1] == "pos_arg3");
    cerr << "test_all_together PASSED" << '\n'
         << '\n';
}


int main() {
    test_positional_only();
    test_with_flags_and_valued_options();
    test_all_together();

    cerr << "All tests passed!" << '\n';

    return 0;
}
