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

void testRequiredPositionals() {
    cerr << "--- Testing required positionals ---\n";
    vector<string> args = {"./getPosMinimal", "req1", "req2"};
    auto           argv = create_argv(args);
    Parser::preprocess(argv.size(), argv.data());

    auto pos1 = Parser::getPositional("pos1", "Required positional 1.");
    auto pos2 = Parser::getPositional("pos2", "Required positional 2.");

    assert(!Parser::runAllPostprocess(true));
    assert(pos1 == "req1");
    assert(pos2 == "req2");

    cerr << "Required positionals PASSED\n"
         << '\n';
}

void testRequiredRemainingPositionals() {
    cerr << "--- Testing required remaining positionals ---\n";
    vector<string> args = {"./getPosMinimal", "req1", "req2", "req3", "req4"};
    auto           argv = create_argv(args);
    Parser::preprocess(argv.size(), argv.data());

    auto pos1   = Parser::getPositional("pos1", "Required positional 1.");
    auto pos2   = Parser::getPositional("pos2", "Required positional 2.");
    auto posVec = Parser::getRemainingPositionals("posVec", "Remaining required positionals.");

    assert(!Parser::runAllPostprocess(true));
    assert(pos1 == "req1");
    assert(pos2 == "req2");
    assert(posVec.size() == 2);
    assert(posVec[0] == "req3");
    assert(posVec[1] == "req4");

    cerr << "Required remaining positionals PASSED\n"
         << '\n';
}

void testOptionalPositionals() {
    cerr << "--- Testing optional positionals ---\n";
    // Test case 1: Optional arguments not provided
    {
        vector<string> args = {"./getPosMinimal", "req1"};
        auto           argv = create_argv(args);
        Parser::preprocess(argv.size(), argv.data());

        auto pos1 = Parser::getPositional("pos1", "Required positional.");
        auto opt1 = Parser::getPositional("opt1", "Optional positional.", false, "default1");
        auto opt2 = Parser::getPositional("opt2", "Optional positional 2.", false, "default2");

        assert(!Parser::runAllPostprocess(true));
        assert(pos1 == "req1");
        assert(opt1 == "default1");
        assert(opt2 == "default2");
    }

    // Test case 2: Optional arguments provided
    {
        vector<string> args = {"./getPosMinimal", "req1", "val1", "val2"};
        auto           argv = create_argv(args);
        Parser::preprocess(argv.size(), argv.data());

        auto pos1 = Parser::getPositional("pos1", "Required positional.");
        auto opt1 = Parser::getPositional("opt1", "Optional positional.", false, "default1");
        auto opt2 = Parser::getPositional("opt2", "Optional positional 2.", false, "default2");

        assert(!Parser::runAllPostprocess(true));
        assert(pos1 == "req1");
        assert(opt1 == "val1");
        assert(opt2 == "val2");
    }

    cerr << "Optional positionals PASSED\n"
         << '\n';
}

void testOptionalRemainingPositionals() {
    cerr << "--- Testing optional remaining positionals ---\n";
    // Test case 1: Optional arguments not provided, using default value
    {
        vector<string> args = {"./getPosMinimal", "req1"};
        auto           argv = create_argv(args);
        Parser::preprocess(argv.size(), argv.data());

        auto pos1   = Parser::getPositional("pos1", "Required positional 1.");
        auto posVec = Parser::getRemainingPositionals("posVec", "Remaining optional positionals.", false, {"d1", "d2"});

        assert(!Parser::runAllPostprocess(true));
        assert(pos1 == "req1");
        assert(posVec.size() == 2);
        assert(posVec[0] == "d1");
        assert(posVec[1] == "d2");
    }

    // Test case 2: Optional arguments provided
    {
        vector<string> args = {"./getPosMinimal", "req1", "val1", "val2", "val3"};
        auto           argv = create_argv(args);
        Parser::preprocess(argv.size(), argv.data());

        auto pos1   = Parser::getPositional("pos1", "Required positional 1.");
        auto posVec = Parser::getRemainingPositionals("posVec", "Remaining optional positionals.", false, {"d1", "d2"});

        assert(!Parser::runAllPostprocess(true));
        assert(pos1 == "req1");
        assert(posVec.size() == 3);
        assert(posVec[0] == "val1");
        assert(posVec[1] == "val2");
        assert(posVec[2] == "val3");
    }

    cerr << "Optional remaining positionals PASSED\n"
         << '\n';
}

void testMixedPositionals() {
    cerr << "--- Testing mixed positionals ---\n";
    vector<string> args = {"./getPosMinimal", "req1", "opt1_val"};
    auto           argv = create_argv(args);
    Parser::preprocess(argv.size(), argv.data());

    auto pos1 = Parser::getPositional("pos1", "Required positional.");
    auto opt1 = Parser::getPositional("opt1", "Optional positional.", false, "default1");
    auto opt2 = Parser::getPositional("opt2", "Optional positional 2.", false, "default2");

    assert(!Parser::runAllPostprocess(true));
    assert(pos1 == "req1");
    assert(opt1 == "opt1_val");
    assert(opt2 == "default2");

    cerr << "Mixed positionals PASSED\n"
         << '\n';
}

void testMixedRemainingPositionals() {
    cerr << "--- Testing mixed remaining positionals ---\n";
    // Test case 1: Optional argument not provided
    {
        vector<string> args = {"./getPosMinimal", "req1"};
        auto           argv = create_argv(args);
        Parser::preprocess(argv.size(), argv.data());

        auto pos1   = Parser::getPositional("pos1", "Required positional.");
        auto opt1   = Parser::getPositional("opt1", "Optional positional.", false, "default1");
        auto posVec = Parser::getRemainingPositionals("posVec", "Remaining optional positionals.", false, {"d1", "d2"});

        assert(!Parser::runAllPostprocess(true));
        assert(pos1 == "req1");
        assert(opt1 == "default1");
        assert(posVec.size() == 2);
        assert(posVec[0] == "d1");
        assert(posVec[1] == "d2");
    }

    // Test case 2: Optional argument provided
    {
        vector<string> args = {"./getPosMinimal", "req1", "opt1_val", "rem1", "rem2"};
        auto           argv = create_argv(args);
        Parser::preprocess(argv.size(), argv.data());

        auto pos1   = Parser::getPositional("pos1", "Required positional.");
        auto opt1   = Parser::getPositional("opt1", "Optional positional.", false, "default1");
        auto posVec = Parser::getRemainingPositionals("posVec", "Remaining optional positionals.", false, {"d1", "d2"});

        assert(!Parser::runAllPostprocess(true));
        assert(pos1 == "req1");
        assert(opt1 == "opt1_val");
        assert(posVec.size() == 2);
        assert(posVec[0] == "rem1");
        assert(posVec[1] == "rem2");
    }

    cerr << "Mixed remaining positionals PASSED\n"
         << '\n';
}

void testMissingRequiredPositional() {
    cerr << "--- Testing missing required positional ---\n";
    vector<string> args = {"./getPosMinimal"};
    auto           argv = create_argv(args);
    Parser::preprocess(argv.size(), argv.data());

    auto pos1 = Parser::getPositional("pos1", "Required positional.");

    assert(Parser::runAllPostprocess(true)); // Expect error

    cerr << "Missing required positional PASSED\n"
         << '\n';
}

int main() {
    testRequiredPositionals();
    testRequiredRemainingPositionals();
    testOptionalPositionals();
    testOptionalRemainingPositionals();
    testMixedPositionals();
    testMixedRemainingPositionals();
    testMissingRequiredPositional();

    cerr << "All tests passed!\n";

    return 0;
}
