[简体中文](./README-zh.md)

- [Overview](#overview)
    - [Minimal Version Features](#minimal-version-features)
    - [Full Version Features](#full-version-features)
- [Usage](#usage)
  - [Basic Usage](#basic-usage)
  - [Workflow](#workflow)
  - [Formatted Output](#formatted-output)
  - [API Reference](#api-reference)
    - [Program Info](#program-info)
    - [Preprocessing](#preprocessing)
    - [Getting Flag Options](#getting-flag-options)
    - [Getting Values Options](#getting-values-options)
      - [`OptValBuilder<T>`](#optvalbuildert)
      - [Parsing Custom Types](#parsing-custom-types)
    - [Getting Positional Arguments](#getting-positional-arguments)
    - [Post-processing](#post-processing)
      - [Help Printing](#help-printing)
      - [Error Handling](#error-handling)
    - [Option Grouping](#option-grouping)
    - [Subcommands](#subcommands)
  - [Other Features](#other-features)
- [Examples](#examples)
- [Benchmarks](#benchmarks)
  - [Compilation Time \& Binary Size](#compilation-time--binary-size)
  - [Runtime Memory Consumption](#runtime-memory-consumption)
  - [Summary](#summary)

# Overview

Why create another command-line argument parsing library when there are already so many? Because I wanted a library that is both lightweight and easy to use, but I couldn't find one that met my needs.

- [CLI11](https://github.com/CLIUtils/CLI11): Comprehensive and easy to use, but too heavy.
- [cxxopts](https://github.com/jarro2783/cxxopts): Claims to be lightweight but is actually quite heavy, and the API is clumsy.
- [args](https://github.com/Taywee/args): Very lightweight, but the API is not user-friendly.
- [argh](https://github.com/adishavit/argh): Lightweight enough but too primitive; it doesn't even generate help messages automatically.
- [argparse](https://github.com/morrisfranken/argparse): Lightweight, easy to use, and cleverly implemented. It was my favorite, but its help message format is unconventional, and it lacks support for syntax like `-n123`.

So, I wrote **ArgLite**: a truly lightweight, easy-to-use, and feature-rich library that produces modern, aesthetically pleasing help and error messages. See the [Benchmarks](#benchmarks) section for details on its weight. Regarding "ease of use," it follows standard intuitions:

1.  **Clear Naming**: Use clearly named functions to add and retrieve arguments, not overloaded operators.
2.  **Concise**: Retrieve an argument in a single statement. Unlike most libraries where you declare a variable, bind it, and then parse, or register and then fetch, (often requiring code changes in multiple places to add one argument), ArgLite does it all in one go.

This library is not for those who need some unusual usages, including:

1. Multi-threaded argument parsing
2. Associating multiple short or long options with the same option. For example, `-f, -i, --file, --input` all referring to the same option.
3. Parsing positional arguments before options,

ArgLite comes in two versions: **Minimal** and **Full**. Simply `#include "ArgLite/Minimal.hpp"` or `#include "ArgLite/Core.hpp"` to use the respective version. It is a header-only library, so just include and go.

The **Minimal** version maintains extreme lightness with basic functionality. The **Full** version supports advanced features; it's more complex but still lighter than most alternatives. While the Full version doesn't match the breadth of "kitchen-sink" libraries like CLI11, it adheres to the C++ philosophy of "You don't pay for what you don't use" to a certain extent.

- **Minimal**: Low overhead, basic features, intuitive API. Ideal for simple needs.
- **Full**: Extended with key advanced features at low overhead. Ideal for moderately complex programs.

### Minimal Version Features

- Positional arguments: `arg1 arg2 ...`
- Flags: `-v`, `--verbose`
- Options with values: `-f path`, `--file=path`, `-fpath`
- Combined short options: `-abc` (equivalent to `-a -b -c`)
- Combined short options with value: `-abcf file` (equivalent to `-a -b -c -f file`)
- So for flags `-v, --verbose`, `-a` and the option `-f, --file` requiring a value, the following usages are equivalent:
  - `--verbose -a --file path`
  - `-va --file=path`
  - `-vaf path`
  - `-vafpath`
- Flexible syntax: `-va --file=path`, `-vaf path`, and `-vafpath` are all valid.
- Flexible positionals: Positional arguments don't need to be at the end, just not immediately after an option that expects a value.
    - `-v -n 123 file1 file2` and `-v file1 -n 123 file2` are equivalent.
- Mutually exclusive options: `--enable-feature`, `--disable-feature`
- End-of-options delimiter: `--` treats all subsequent arguments as positionals.
- Types: `bool`, `long long`, `double`, `std::string`
- Default values for options and positionals.
- Automatic help generation (`-h, --help`).
- Option grouping in help messages.
- **Formatted Output**: Colored and bold text for better-looking help and error messages.

### Full Version Features

Includes all Minimal features, plus:

- Subcommands.
- Flag counting: `-vvv`.
- Multi-value options: `-f file1 -f file2`, `-f file1,file2`.
-
- `std::optional` support to distinguish between "not provided" and "default value".
- Custom value type names in help.
- Mandatory options.
- Chainable calls for configuration (see [OptValBuilder](#optvalbuildert)).

**Recommendation**: The Full version offers more features but has a smaller advantage over existing libraries compared to the Minimal version. If you are considering this library, I strongly recommend looking at the **Minimal** version. It is arguably the lightest library available that provides both parameter parsing and help generation. It is perfect for small programs where you want easy-to-use parsing with minimal cost. Yes, there are indeed some lighter libraries (such as argh mentioned above), but their functionality is quite basic, and you may need to write more code related to command-line argument parsing.

# Usage

## Basic Usage

This project provides well-encapsulated examples, enabling you to complete command-line argument parsing tasks by modifying them, without needing to delve deeply into the specific usage of the library. For a quick start, we recommend checking out [simple_wrapped.cpp](./examples/simple_wrapped.cpp). More details can be found in the [Examples](#examples) section.

This section provides a concise overview of how to use the Minimal version. The interfaces for both versions are very similar, making migration easy. This example is only for basic usage demonstration; please refer to the [API Reference](#api-reference) section for specific usages of these APIs.

```cpp
#include "ArgLite/Minimal.hpp"

using namespace std;
using ArgLite::Parser;

int main(int argc, char **argv) {
    // Step 1: Preprocess
    Parser::preprocess(argc, argv);

    // Step 2: Retrieve command line arguments
    auto verbose    = Parser::hasFlag("v,verbose", "Enable verbose output."); // Short and long
    auto number     = Parser::getInt("number", "Number of iterations.");      // Long only
    auto rate       = Parser::getDouble("r", "Rate.", 123.0);                 // Short only with default
    auto outputPath = Parser::getString("o,out-path", "Output file Path.", ".");
    auto outputFile = Parser::getPositional("output-file", "The output file name.");
    auto inputFiles = Parser::getRemainingPositionals("input-files", "The input files to process.");

    // Step 3: Postprocess
    Parser::runAllPostprocess();

    cout << "Verbose    : " << boolalpha << verbose << '\n';
    cout << "Number     : " << number << '\n';
    cout << "Rate       : " << rate << '\n';
    cout << "Output Path: " << outputPath << '\n';
    cout << "Output file: " << outputFile << '\n';
    cout << "Input files:" << '\n';
    for (const auto &it : inputFiles) { cout << "  " << it << '\n'; }

    return 0;
}
```

Program output:

```
> ./app --help
Usage: app [OPTIONS] output-file input-files...

Positional Arguments:
  output-file  The output file name.
  input-files  The input files to process.

Options:
  -v, --verbose          Enable verbose output.
      --number <integer>
                         Number of iterations. [default: 0]
  -r <float>             Rate. [default: 123.0]
  -o, --out-path <string>
                         Output file Path. [default: .]
  -h, --help             Show this help message and exit

> ./app --number 666 -v -r 2.333 -o ./results out.txt in1.txt in2.txt
Verbose    : true
Number     : 666
Rate       : 2.333
Output Path: ./results
Output file: out.txt
Input files:
  in1.txt
  in2.txt
```

The API is designed to be self-explanatory. You set program info, preprocess, retrieve values into variables, and then post-process. After post-processing, you have all the values you need, so use them as you like.

**Key Difference**: Unlike other libraries where you register options and *then* parse/fetch, ArgLite allows you get the value immediately upon registration. `hasFlag` and `getXxx` both register the option and return its value simultaneously. We call this "**Register and Get**".

Although the above example uses variables to store the return values, you can actually use constants as well. Because all functions return an rvalue. This is a feature of ArgLite. Other libraries typically can't directly store parsing results in constants; you usually have to store the result in a variable first, and then use that variable to initialize the constant.

```cpp
// You can store the result in a constant
const auto number = Parser::getInt("number", "Number of iterations.");
```

The **Full version** is similar but replaces `getType()` with `get<T>()`, supporting more types and a fluent interface. See [Getting Values](#getting-values-options) for details.

## Workflow

If you're only interested in using this library and not its working details, feel free to skip this section. This project offers well-encapsulated examples that you can easily copy, paste, and adapt. Check out the [Examples](#examples) section for more information.

Certain interfaces require a specific order. Below is a standard, safe workflow (using the Full version syntax):

**Basic Steps:**

1.  Program Info (Optional)
2.  Subcommands (Optional)
3.  Define Short Options with Values (Optional, specialized)
4.  **Preprocess**
5.  **Get Options** (Flags and Values, then Positionals)
6.  **Postprocess**

**Code Example:**

```cpp
using ArgLite::Parser;
using ArgLite::SubParser;

// === Program Info (Optional)
Parser::setDescription("A simple program to demonstrate ArgLite subcommand feature.");
Parser::setVersion("1.2.3");

// === Subcommands (Optional)
SubParser status("status", "Show the working tree status");
SubParser commit("commit", "Record changes to the repository");

// === Define Short Options with Values (Optional)
// Tells the parser that '-i' takes a value, necessary for parsing '-i123' correctly.
Parser::setShortNonFlagOptsStr("i");
commit.setShortNonFlagOptsStr("mF");

// === Preprocess
Parser::preprocess(argc, argv);

// === Get Options
auto verbose    = Parser::countFlag("v,verbose", "Enable verbose output.");
auto indent     = Parser::get<int>("i,indent", "Option Description indent.").setDefault(26).setTypeName("num").get();

// Positionals must be retrieved after options.
auto outputFile = Parser::getPositional("output-file", "The output file name.");
// Remaining positionals must be retrieved last.
auto inputFiles = Parser::getRemainingPositionals("input-files", "The input files to process.");

// Order between main command and subcommands doesn't matter, as they are independent.
auto commitAll      = commit.hasFlag("a,all", "Commit all changes.");
auto commitMsg      = commit.get<string>("m,message", "Use the given <msg> as the commit message.").get();
auto commitDate     = commit.get<int>("date", "Override the author date used in the commit.").get();
auto commitPathSpec = commit.getRemainingPositionals("pathspec", " When pathspec is given on the command line, ...", false);

// === Postprocess
Parser::changeDescriptionIndent(indent); // optional
Parser::runAllPostprocess();
```

Thinking this way might help you understand this library: `preprocess()` is like `parse()` in other libraries; the `get` functions are like `add_option()` in other libraries, but they also retrieve values. Subcommands are definitions and come before preprocessing.

**Note**: The library is designed to parse once and exit. Internal data is cleared to save resources. If you are not using subcommands, do not call library functions after `runAllPostprocess()`. If using subcommands, only `Parser::isMainCmdActive()` and `.isActive()` are valid after post-processing.

## Formatted Output

```cpp
#define ARGLITE_ENABLE_FORMATTER
```

Define this macro to enable ANSI color sequences for terminals (bold, colors). These sequences are automatically suppressed if output is redirected to a pipe or file.

Most C++ libraries ignore this, but ArgLite (even the Minimal version) supports it out of the box. However, due to the simple implementation, it will print ANSI sequences directly instead of formatted text on the old Windows terminal. Therefore, if you are building applications for older Windows platforms, it might be better not to enable this feature.

Screenshots:

![](https://raw.githubusercontent.com/InMirrors/images/main/ArgLite/formatter-help.png)
![](https://raw.githubusercontent.com/InMirrors/images/main/ArgLite/formatter-error.png)

## API Reference

All external interfaces are documented with Doxygen comments. This readme and the comments complement each other, meaning the documentation may omit some details that are present in the comments. If you've read the documentation and still have questions about how to use a particular interface, please refer to the comments.

All needed interfaces are provided as static functions within `ArgLite::Parser` if you don't use subcommands. The Full version adds `ArgLite::SubParser` for subcommands. An interface that is only available in a specific version will be noted. If there is no note, it means the interface is available in both versions and the usage is the same, although the internal implementation may differ.

All `optName` parameters are short option names or long option names (without the `-` or `--` prefixes). It can also be both (short option name first, followed by the long option name, separated by a `,`), for example, `o`, `output`, `o,output`. All `description` parameters are used for display in help messages.

### Program Info

Usually called at the beginning, but valid anytime before post-processing.

```cpp
void setDescription(std::string description);
```

Sets the program description shown on the first line of the help message.

---

```cpp
void setVersion(std::string versionStr);
```
Sets the version and enables `-V` and `--version`.

### Preprocessing

```cpp
void setShortNonFlagOptsStr(std::string shortNonFlagOptsStr);
```
**Optional**. Use this only if you need to support the syntax `-n123` where `n` is an option that takes a value. If you're new to this library, it's recommended to ignore this.

During preprocessing, the library doesn't know if `-n123` means `-n -1 -2 -3` (flags) or `-n 123` (value). It defaults to flags. To treat `n` as taking a value, you must register it here string of characters (e.g., `"n"`).
*Note: Do not include flag options here.*

This is the least elegant part of the library, but necessary for this specific syntax. To avoid missing any, you can use Regex to find them in your code:

```js
// Main command
(?<=::get.+?\(").(?=[,"])
// Subcommand (replace `subcmd` with the subcommand variable name)
(?<=subcmd\.get.+?\(").(?=[,"])
```

---

```cpp
void preprocess(int argc, const char *const *argv);
```
**Required**. The entry point for the library. Must be called before getting values.

### Getting Flag Options

```cpp
bool hasFlag(std::string_view optName, std::string description);
```
Returns `true` if the flag is present.

---

```cpp
struct HasMutualExArgs {
    std::string trueOptName;
    std::string trueDescription;
    std::string falseOptName;
    std::string falseDescription;
    bool        defaultValue;
};
bool hasMutualExFlag(HasMutualExArgs args);
```

Handles two mutually exclusive flags (e.g., `--enable` vs `--disable`).
- Returns `true` if the first is present.
- Returns `false` if the second is present.
- Returns `defaultValue` if neither is present.
- If both are present, the last one wins.

Usage (C++20 designated initializers):
If you're using C++20 or later, it's recommended to use designated initializers:
```cpp
auto enableX = Parser::hasMutualExFlag({
     .trueOptName      = "x,enable-x",
     .trueDescription  = "Enable feature x.",
     .falseOptName     = "X,disable-x",
     .falseDescription = "Disable feature x.",
     .defaultValue     = false
});
```

---

```cpp
unsigned countFlag(std::string_view optName, std::string description);
```
**Full Version Only**. Counts occurrences (e.g., `-vvv` returns 3), incrementing the counter by 1 for both long and short options.

### Getting Values Options

```cpp
// Minimal Version
// The signatures of `getInt`, `getDouble`, `getString`, and `getBool` are similar, differing only in their specific types.
T getType(std::string_view optName, const std::string &description, T defaultValue = T{});

// Full Version
template <typename T>
OptValBuilder<T> get(std::string_view optName, std::string description);
```

**Minimal**: Returns the value directly. Supports `int`, `double`, `string`, `bool`. For other types, get a string and convert it manually.
**Full**: Returns an `OptValBuilder<T>`. Supports almost all built-in types and `std::optional`.

**Bool parsing rules** (case-insensitive):
- True: `1`, `true`, `yes`, `on`
- False: `0`, `false`, `no`, `off`

#### `OptValBuilder<T>`

**Full Version Only**.

Methods available on the builder object:

- `setDefault(T defaultValue)`: Set a default value.

- `setTypeName(std::string typeName)`: Change the type name in help (e.g., `--file <path>`).

- `required()`: Make the option mandatory.

- `T get()`: Retrieve a single value.

- `std::vector<T> getVec(char delimiter = '\0')`: Retrieve multiple values.
    - `-f file1 -f file2` -> `[file1, file2]`
    - With `getVec(',')`: `-f file1,file2` -> `[file1, file2]`

#### Parsing Custom Types

**Full Version Only**.

ArgLite prioritizes being lightweight and does **not** provide full support for custom types, as that would require heavy template metaprogramming, significantly increasing the library's size. However, it offers limited support for handling custom types, mainly by providing help and error messages.

If you need comprehensive support for custom types, please choose another library. The core philosophy of this library is to be lightweight. Advanced features are supported only if they can be implemented in a lightweight manner. Supporting custom types involves not only parsing and validation but also generating appropriate help and error messages, which would make the library much heavier.

This section explains how to use the limited support to handle custom types.

---

**Setting Error Messages**

For unknown types, `get<T>()` will attempt to initialize the type with the parsed string. If your custom type can be initialized from a string, you can theoretically use it to get the parsing result. However, it cannot provide validation, and the help and error messages cannot provide relevant information. Therefore, the following method using `get<std::string>()` is more recommended.

First, get the raw string of the argument, then write your parsing and validation logic. If validation fails, call this function to insert an error message.

```cpp
void pushBackErrorMsg(std::string msg);
```

It pushes the provided string into an internal array for storing error messages, which is used in the post-processing stage.

For example, in the [subcommand example](./examples/subcommand.cpp#L52-L70), the `--color` option of the `grep` subcommand accepts "auto", "always", or "never". This code snippet validates the option's value:

```cpp
auto grepColor = grep.get<string>(...).get();
// Validate grep color option value.
if (grepColor != "auto" && grepColor != "always" && grepColor != "never") {
    string errorMsg("Invalid value for option '--color'. Expected 'auto', always' or 'never', but got '");
    errorMsg += grepColor;
    errorMsg += "'.";
    grep.pushBackErrorMsg(errorMsg);
}
```

When an invalid argument is passed, it will print:

```
>./subcommand grep --color blue
Errors occurred while parsing command-line arguments.
The following is a list of error messages:
Error: Invalid value for option '--color'. Expected 'auto', always' or 'never', but got 'blue'.
```

If you need finely formatted text like ArgLite's output, you can call the interfaces under `ArgLite::Formatter`. There are `red()`, `yellow()`, `bold()`, and `boldUnderline()` functions, with similar signatures.

```cpp
std::string_view red(std::string_view str, const std::ostream &os = std::cerr);
std::string red(std::string_view str, const std::ostream &os = std::cerr); // #ifdef ARGLITE_ENABLE_FORMATTER
```

When the `ARGLITE_ENABLE_FORMATTER` macro is not defined, these functions directly return a view of the input string. When the macro is defined, they return a formatted string and do not add ANSI sequences when outputting to a file or pipe.

The second parameter is the output stream used for detection; the function determines whether it is outputting to a terminal based on this stream. The default value of the second parameter for `bold()` and `boldUnderline()` is `std::cout`, but error messages are output to `std::cerr`, so you need to pass `std::cerr` as the second parameter when using them here. You can also uniformly pass `std::cerr` as the second parameter to avoid remembering their default values.

Writing it like in the [subcommand example](./examples/subcommand.cpp#L62-L70) will produce this output:

![](https://raw.githubusercontent.com/InMirrors/images/main/ArgLite/formatter-custom-types.png)

You only need to call `pushBackErrorMsg` before post-processing to print error messages when the user inputs invalid arguments. However, if you have requirements for the order of error messages, you need to call it before the next call to an interface of ArgLite that gets command-line arguments, otherwise the error message from the next call will be placed first.

---

**Setting Help Messages**

Since ArgLite does not recognize custom types, you need to manually adjust the help message if you want to specify what values an option accepts. The code in the [subcommand example](./examples/subcommand.cpp#L57-L61):

```cpp
auto grepColor = grep.get<string>("color", "When to use colors. [possible values: auto, always,\n"
                                           "never].")
                     .setDefault("auto")
                     .setTypeName("when")
                     .get();
```

produces:

```
      --color <when>      When to use colors. [possible values: auto, always,
                          never]. [default: auto]
```

Alternatively, you can hint at valid arguments in the value's type name instead of the description:

```cpp
auto grepColor = grep.get<string>("color", "When to use colors.")
                     .setDefault("auto")
                     .setTypeName("auto|always|never")
                     .get();
```

```
      --color <auto|always|never>
                          When to use colors. [default: auto]
```

### Getting Positional Arguments

```cpp
std::string getPositional(
    const std::string &posName, std::string description, bool required = true,
    std::string defaultValue = "");
std::vector<std::string> getRemainingPositionals(
    const std::string &posName, std::string description, bool required = true,
    const std::vector<std::string> &defaultValue = {});
```

ArgLite provides interfaces for getting a single positional argument and for getting all remaining positional arguments. These must be called **after** all option-related calls (i.e., the interfaces from the previous two subsections). The consumption of positional arguments is sequential, similar to other libraries: the first call consumes the first available positional argument from the command line.

Some libraries support getting a single positional argument after getting multiple ones, enabling a pattern like:

```bash
command input1 input2... output
```

ArgLite does **not** support this pattern. Because it returns the parsing result immediately, `getPositional()` consumes one positional argument, and `getRemainingPositionals()` consumes all of them. To get multiple positional arguments, you must structure your command line like this:

```bash
command output input1 input2...
```

Both interfaces have an optional `required` parameter. Only the last positional arguments can be marked as optional (`required = false`), similar to how default arguments in C++ functions work. You cannot have a required positional argument after an optional one. If an optional positional argument is not provided on the command line, it will take its `defaultValue`.

Since most applications use string types for positional arguments, ArgLite only supports parsing them as strings to keep it lightweight and simple. If you need other types, consider using valued options instead or using another library. If you must use positional arguments for other types with this library, you can get the string and convert it manually. Alternatively, you could modify the source code to expose `ArgLite::Parser::convertType<T>()`, which supports `bool`, `char`, and `std::optional`, types not supported by standard library conversions.

### Post-processing

#### Help Printing

```cpp
void changeDescriptionIndent(size_t indent);
```

Adjusts the indentation of option descriptions in the help message, with a default of 25. This is useful for fine-tuning the help message's appearance.

For example, if many of your option names are slightly too long, their descriptions will wrap to the next line. By increasing the indent, you can make these descriptions align on the same line, resulting in a cleaner look. The simple example in the [Usage](#usage) section is a case in point; setting the indent to 27 would align everything on a single line.

---

```cpp
void setHelpFooter(std::string_view footer);
```

Appends extra information to the help message. The provided string will be printed after the auto-generated help content. You can use this to add usage examples, contact information, etc. If you need finely formatted text like the library's auto-generated help, you can use the interfaces under `ArgLite::Formatter`. See the [Parsing Custom Types](#parsing-custom-types) section for usage.

For instance, the [subcommand example](./examples/subcommand.cpp#L77-L83) includes this code:

```cpp
// Set the help footer
string footer;
footer += ArgLite::Formatter::boldUnderline("Examples:\n");
footer += "  subcommand -v out.txt in1.txt in2.txt\n";
footer += "  subcommand status\n";
footer += "  subcommand commit -m \"An awesome commit\"";
Parser::setHelpFooter(footer);
```

This produces the following output:

```
...
  -V, --version           Show version information and exit
  -h, --help              Show this help message and exit

Examples:
  subcommand -v out.txt in1.txt in2.txt
  subcommand status
  subcommand commit -m "An awesome commit"
```

<details><summary>Click to see the full help message screenshot</summary>

![](https://raw.githubusercontent.com/InMirrors/images/main/ArgLite/formatter-footer.png)

</details>

Unlike the error handling functions in the next subsection, `changeDescriptionIndent()` and `setHelpFooter()` don't strictly need to be called during post-processing. They just need to be called before `tryToPrintHelp()`; you can even call them at the beginning. Calling them here simply makes the workflow easier to understand.

---

```cpp
void tryToPrintHelp();
```

Prints the help message and exits the program if the user provides `-h` or `--help`.

Because option names require a prefix `-` and can have both short and long forms, the indentation for their descriptions is significantly larger than for positional arguments or subcommands. This makes it more likely for option descriptions to exceed the terminal width and wrap to the next line. Terminals do not indent the wrapped line, causing the description to start at the beginning of the line, which should be the area for options. This results in a mix of descriptions and options, making it difficult to distinguish and impacting the user experience. For example, in the [subcommand example](./examples/subcommand.cpp):

```cpp
    auto commitSignOff  = commit.hasMutualExFlag({
        "s,signoff",
        "Add a Signed-off-by trailer by the committer at the end of the commit log message.",
        "no-signoff",
        "Do not add a Signed-off-by trailer by the committer at the end of the commit log message.",
        false,
    });
```

The output in the help message would be:

```
  -s, --signoff           Add a Signed-off-by trailer by the committer at theend of the commit log message.
      --no-signoff        Do not add a Signed-off-by trailer by the committerat the end of the commit log message. (default)
```

If you add a `\n` to the description string, for example:

```cpp
    auto commitSignOff  = commit.hasMutualExFlag({
        "s,signoff",
        "Add a Signed-off-by trailer by the committer at the\n"
         "end of the commit log message.",
        "no-signoff",
        "Do not add a Signed-off-by trailer by the committer\n"
         "at the end of the commit log message.",
        false,
    });
```

When printing the help message, subsequent lines will be indented, ensuring that the description content always stays in the right-hand area, resulting in the following output:

```
  -s, --signoff           Add a Signed-off-by trailer by the committer at the
                          end of the commit log message.
      --no-signoff        Do not add a Signed-off-by trailer by the committer
                          at the end of the commit log message. (default)
```

While automatic description splitting would be better, an intelligent implementation would introduce a lot of code, which is inconsistent with the lightweight positioning of the library. Moreover, manually adding a newline character is not complicated and can make the code's display more closely resemble the help output. Therefore, this library adopts this less intelligent implementation.

#### Error Handling

```cpp
bool tryToPrintInvalidOpts(bool notExit = false);
```
Checks for and reports any unknown options.

```cpp
bool finalize(bool notExit = false);
```
Finalizes parsing and prints all accumulated error messages, such as missing values for options, incorrect argument types, or missing positional arguments. This function will clear a large amount of internal intermediate data. Therefore, after running this function, you should not use any other library interfaces, except for checking the active command when using subcommands. At this point, the library has returned all parsing results, and you should only use the values you've already stored.

```cpp
bool runAllPostprocess(bool notExit = false);
```
Runs `tryToPrintHelp()`, `tryToPrintInvalidOpts()`, and `finalize()` in sequence.

Typically, you just need to call `runAllPostprocess()`. The functions it calls will exit the program upon encountering an error. If you don't want to exit immediately, pass `true`, and the function will return `true` if an error occurs. If you need more fine-grained control, you can manually call these functions. For example, not exiting for unknown options but exiting for parsing errors, you can run `tryToPrintHelp(); auto hasInvalidOpts = tryToPrintInvalidOpts(true); finalize();` sequentially.

This library does not throw exceptions; it uses return values to indicate errors. Since errors in a lightweight library like this are typically due to incorrect command-line arguments that require the user to re-enter them, return values are sufficient and align with the lightweight design.

### Option Grouping

```cpp
void insertOptHeader(std::string header);
```

Options in the help message are arranged in the order they are registered, with each registration adding an entry. The approach to adding option groups is similar: use this function to insert a group header. When printing the help message, the function automatically adds a newline and a colon to the header, maintaining the same style as the default `Options:` title. After calling this function, the original `Options:` title will disappear, so you need to add your desired titles yourself.

Below is an example that mimics the help message of [ripgrep](https://github.com/BurntSushi/ripgrep). You can find the [full code](./examples/option_grouping.cpp) in the examples folder.

```cpp
Parser::preprocess(argc, argv);

Parser::insertOptHeader("Input Options");
auto regexp = Parser::getString("e,regexp", "A pattern to search for.");
auto file   = Parser::getString("f,file", "Search for patterns from the given file.");

Parser::insertOptHeader("Search Options");
auto ignoreCase = Parser::hasMutualExFlag({
    "i,ignore-case",
    "Case insensitive search.",
    "s,case-sensitive",
    "Search case sensitively",
    false,
});
auto maxCount   = Parser::getInt("m,max-count", "Limit the number of matching lines.");

// Add a header for "-h, --help" and "-V, --version"
Parser::insertOptHeader("Other Behaviors:");

Parser::runAllPostprocess();
```

This will produce the following help message:

```
Input Options:
  -e, --regexp <string>  A pattern to search for.
  -f, --file <string>    Search for patterns from the given file.

Search Options:
  -i, --ignore-case      Case insensitive search.
  -s, --case-sensitive   Search case sensitively (default)
  -m, --max-count <integer>
                         Limit the number of matching lines. [default: 0]

Other Behaviors:
  -V, --version          Show version information and exit
  -h, --help             Show this help message and exit
```

If no `insertOptHeader()` statements are added, the help message would be:

```
Options:
  -e, --regexp <string>  A pattern to search for.
  -f, --file <string>    Search for patterns from the given file.
  -i, --ignore-case      Case insensitive search.
  -s, --case-sensitive   Search case sensitively (default)
  -m, --max-count <integer>
                         Limit the number of matching lines. [default: 0]
  -V, --version          Show version information and exit
  -h, --help             Show this help message and exit
```

You might find this implementation a bit crude, as it doesn't truly bundle options into groups but simply inserts a help entry. In reality, grouping in counterparts generally only affects the display in the help message; the storage and usage of individual options are not fundamentally different from when they are not grouped. You don't access them like `args["group"]["opt"]`. Since this library aims to be lightweight, it implements features in the simplest way possible, even if the method is unconventional.

### Subcommands

**Full Version Only**. Define subcommands by creating `SubParser` objects.

ArgLite follows a simple rule: **"Static methods for the main command, member methods for subcommands."** This means you won't create any objects unless you're using subcommands. Most counterparts require creating an object to parse command-line arguments, but ArgLite uses a static-first design for lightness and ease of use. Since most applications parse arguments only once, static methods and data are sufficient.

ArgLite supports only a single level of subcommands, not nested ones. Programs requiring multiple levels of subcommands are not the target audience for this lightweight library.

Why only one level? The class structure itself has a hierarchy: static members are at the bottom level (one instance), while member instances are at a higher level (multiple instances). This structure perfectly mirrors a single-level subcommand system, making it easy to implement. The `Parser` and `SubParser` objects themselves have no hierarchical relationship; they leverage language features to create a two-level tree structure at a low cost. Implementing higher-level subcommands would significantly increase the library's complexity and size. Therefore, multi-level subcommands are not currently supported and likely won't be in the future.

For detailed usage, see the [example](examples/subcommand.cpp), which also demonstrates most of the Full version's features.

---

```cpp
SubParser(std::string subCommandName, std::string subCmdDescription);
```

Constructor. `subCommandName` is the name of the subcommand, and `subCmdDescription` is its description. Both are displayed in the help message.

---

```cpp
bool isActive();
```

Checks if the subcommand is active. Since registering an option returns a value regardless of which command is active (returning the type's default value if inactive), this member function is provided to check for activation. `bool Parser::isMainCmdActive()` is also available to check if the main command is active.

---

The `SubParser` object also has methods like `setShortNonFlagOptsStr`, `hasFlag`, `countFlag`, `hasMutualExFlag`, `get`, `getPositional`, and `getRemainingPositionals`. Their usage is identical to the versions in `Parser`, but they only take effect when the subcommand is active.

## Other Features

Unlike most libraries that require you to define all options before parsing and retrieving values, ArgLite allows you to get the value as soon as you define an option. This enables dynamic behavior where you can use a parsed value to influence subsequent parsing or configuration.

```cpp
#include "ArgLite/Core.hpp"

using namespace std;
using ArgLite::Parser;

int main(int argc, char **argv) {
    Parser::setShortNonFlagOptsStr("iId");
    Parser::preprocess(argc, argv);

    auto indent    = Parser::get<int>("i,indent", "Option Description indent.").setDefault(26).setTypeName("num").get();
    auto delimiter = Parser::get<char>("d,delimiter", "--include delimiter.").setDefault(':').get();
    auto include   = Parser::get<string>("I,include", "Include directory.").setDefault("include").getVec(delimiter);

    Parser::changeDescriptionIndent(indent);
    Parser::runAllPostprocess();

    cout << "Indent     : " << indent << '\n';
    cout << "Delimiter  : '" << delimiter << "'\n";
    cout << "Include:\n";
    for (const auto &it : include) { cout << "  " << it << '\n'; }

    return 0;
}
```

In this example, you can use the `-i, --indent` option to control the indentation of the help message.

```powershell
> app -h
Usage: app [OPTIONS]

Options:
  -i, --indent <num>      Option Description indent. [default: 26]
  -d, --delimiter <char>  --include delimiter. [default: :]
  -I, --include <string>  Include directory. [default: include]
  -h, --help              Show this help message and exit


> app -h -i20
Usage: app [OPTIONS]

Options:
  -i, --indent <num>
                    Option Description indent. [default: 26]
  -d, --delimiter <char>
                    --include delimiter. [default: :]
  -I, --include <string>
                    Include directory. [default: include]
  -h, --help        Show this help message and exit
```

While this specific use case might seem trivial, it demonstrates the feature. Now for a more practical example—dynamically changing the delimiter for a multi-value option.

```powershell
> app -I "path1:path2"
Indent     : 26
Delimiter  : ':'
Include:
  path1
  path2

> app -I "path1 path2"
Indent     : 26
Delimiter  : ':'
Include:
  path1 path2

> app -I "path1 path2" -d " "
Indent     : 26
Delimiter  : ' '
Include:
  path1
  path2
```

If the arguments cannot be handled by a single fixed delimiter, you can use this feature to dynamically control the delimiter and select one that is appropriate for the current arguments.

# Examples

**[simple.cpp](./examples/simple.cpp)**

A basic example demonstrating the basic usage of the Minimal version.

**[subcommand.cpp](./examples/subcommand.cpp)**

A complex example that uses subcommands and showcases almost all the features of the Full version, mimicking Git's argument parsing to demonstrate how ArgLite can handle sophisticated command-line parsing tasks.

**[option_grouping.cpp](./examples/option_grouping.cpp)**

Demonstrates how to use the option grouping feature to create help messages similar to ripgrep's.

**[simple_wrapped.cpp](./examples/simple_wrapped.cpp)** & **[subcommand_wrapped.cpp](./examples/subcommand_wrapped.cpp)**

The preceding examples focus on demonstrating the library's features and may not be ideal as practical application templates. These two examples showcase a more robust approach by wrapping parameters in a class, allowing parsing results to be accessed through an object. By leveraging C++ class initialization order and the singleton pattern, this approach enforces the correct workflow, ensuring that API calls are made in the required order and that parsing occurs only once. This design pattern makes adding new parameters and subcommands as simple as modifying a configuration file.

You can simply copy the template code, modify the program information, parameters, and subcommands to suit your needs, without needing to worry about the library's internal workflow. With these examples, you can become a productive "copy-paste" programmer.

# Benchmarks

In this section, the `argparse` library tested is not the common [p-ranav/argparse](https://github.com/p-ranav/argparse), but [morrisfranken/argparse](https://github.com/morrisfranken/argparse). The former has an API similar to Python's argparse and cxxopts, using strings to query parsing results. This approach is convenient for dynamically typed languages like Python but not for statically typed languages like C++, which require extra code to retrieve results and lack autocompletion support. Therefore, only the more well-known cxxopts of a similar type was tested here. morrisfranken/argparse just shares the name but has vastly different external APIs and internal implementations, offering a more user-friendly interface.

When using `cxxopts` with a reasonably modern compiler, it defaults to including `<regex>`, making the library very heavy. Its description says "Lightweight C++ command line option parser," but its documentation doesn't mention that it includes heavy libraries. Using it directly results in significantly higher compilation times and binary sizes compared to other libraries. Therefore, the tests were conducted with `CXXOPTS_NO_REGEX` defined; otherwise, its results would be very poor.

Except for CLI11, all other libraries are header-only. CLI11 also has a header-only version, but using it would yield poor test results, so the conventional version that requires linking a library file was used. This also makes CLI11 less convenient to integrate than other libraries.

The test measures the overhead of each library when implementing basic functionalities. The tested features include: getting a flag option, getting an integer, getting a single positional argument, and getting the remaining positional arguments. The test files are located in [benchmark](./tests/benchmark/). You can compare the contents of the various `.cpp` files to see the ease of use of each library's API. After comparison, I believe most people will agree with this conclusion: ArgLite, argparse, and CLI11 are very easy to use, while args and cxxopts require more code and are less intuitive.

All libraries were tested using their latest versions as of 2025-12-21 on WSL. The specific results may vary significantly due to the testing platform, environment, library versions, and other factors. However, the relative differences between the libraries should be quite stable and not overturn the conclusions.

## Compilation Time & Binary Size

Except for "ArgLite Sub", all other entries are simple programs that implement "getting a flag option, getting an integer, getting a single positional argument, and getting the remaining positional arguments" for horizontal comparison. "ArgLite Sub" is the result of the [example](./examples/subcommand.cpp) that uses almost all the interfaces of the Full version, representing a much more complex program for vertical comparison. "ArgLite Full" is the test result of a simple program using the Full version, not the full example.

The following results were obtained with the compilation flags `-s -DNDEBUG`.

| Name          | Time (s) | Size (KB) | Time (%) | Size (%) |
| ------------- | -------: | --------: | -------: | -------: |
| ArgLite Mini  |     0.94 |     110.2 |   100.0% |   100.0% |
| ArgLite Full  |     1.08 |     134.2 |   114.9% |   121.8% |
| *ArgLite Sub* |     1.21 |     162.3 |   128.7% |   147.3% |
| CLI11         |     1.46 |     550.3 |   155.3% |   499.4% |
| cxxopts       |     1.98 |     278.3 |   210.6% |   252.5% |
| args          |     1.65 |     278.3 |   175.5% |   252.5% |
| argparse      |     1.28 |     130.3 |   136.2% |   118.2% |

The following results were obtained with the compilation flags `-s -DNDEBUG -O2`.

| Name          | Time (s) | Size (KB) | Time (%) | Size (%) |
| ------------- | -------: | --------: | -------: | -------: |
| ArgLite Mini  |     1.36 |      54.3 |   100.0% |   100.0% |
| ArgLite Full  |     1.56 |      62.3 |   114.7% |   114.7% |
| *ArgLite Sub* |     2.11 |      90.3 |   155.1% |   166.3% |
| CLI11         |     1.75 |     498.3 |   128.7% |   917.7% |
| cxxopts       |     2.67 |     118.3 |   196.3% |   217.9% |
| args          |     2.91 |     150.3 |   214.0% |   276.8% |
| argparse      |     1.77 |      74.3 |   130.1% |   136.8% |

As you can see, ArgLite is indeed the most lightweight, with the Minimal version having a particularly noticeable advantage. Even the full example using various interfaces, ArgLite's performance can still be compared with the simplest programs of other libraries, and even perform better. `argparse` also performs well, just slightly behind ArgLite Full, placing it in the same tier. CLI11 benefits from linking a pre-built library file, resulting in a shorter compilation time (still not as fast as ArgLite), but it has the largest binary size. `cxxopts` and `args` perform moderately, being only lighter than the all-in-one library CLI11, and because they cannot link pre-built library files, their compilation times are the longest.

## Runtime Memory Consumption

In real-world applications, command-line arguments do not involve a huge number of options, nor do programs define a vast number of them. An application with dozens of options is already considered complex. However, parsing a few dozen options is a very light task, and even a low-performance language like Python can complete it in an extremely short time with minimal memory consumption. Positional arguments, on the other hand, can be numerous, as many programs use remaining positional arguments to get files. Even with thousands of files, this is still a very lightweight task. Therefore, testing the runtime performance of such a library doesn't have much practical significance. This test is done just for a more comprehensive comparison of these libraries. In most cases, you don't need to decide which library is more suitable based on these test results. Even the worst-performing one would not be noticeably worse in practical applications.

This test uses `valgrind`'s `memcheck` and `massif` tools, and the tested binaries are the ones compiled with `-s -DNDEBUG -O2` from the previous test. Since the C++ runtime itself occupies a certain amount of memory, a "Hello World" program is included as a baseline. The results for each library are the values after subtracting this baseline. The basic arguments passed are `-v -c 123 outfile infile1 infile2`, with additional tests for more positional arguments. The test results include: the number of heap memory allocations, total allocated size, and peak heap size.

| Name         | Allocs | Allocated (KB) | Peak Heap (KB) |
| ------------ | -----: | -------------: | -------------: |
| Baseline     |      2 |          75.00 |          75.02 |
| ArgLite Mini |     20 |           1.37 |           0.16 |
| ArgLite Full |     22 |           1.45 |           0.16 |
| CLI11        |    133 |           8.14 |           5.96 |
| cxxopts      |    109 |           9.94 |           8.25 |
| args         |     41 |           2.19 |           0.96 |
| argparse     |     54 |           4.37 |           3.10 |

Base arguments + 1000 positional arguments:

| Name         | Allocs | Allocated (KB) | Peak Heap (KB) |
| ------------ | -----: | -------------: | -------------: |
| Baseline     |      2 |          75.00 |          75.02 |
| ArgLite Mini |     37 |          73.21 |          48.88 |
| ArgLite Full |     39 |          73.29 |          48.88 |
| CLI11        |    158 |         183.01 |         121.18 |
| cxxopts      |    125 |         232.06 |         134.98 |
| args         |     50 |          97.31 |          76.38 |
| argparse     |     76 |         254.48 |         160.98 |

Base arguments + 10000 positional arguments:

| Name         | Allocs | Allocated (KB) | Peak Heap (KB) |
| ------------ | -----: | -------------: | -------------: |
| Baseline     |      2 |          75.00 |          75.02 |
| ArgLite Mini |     45 |        1153.21 |         828.88 |
| ArgLite Full |     47 |        1153.29 |         828.88 |
| CLI11        |    170 |        2624.26 |        1722.10 |
| cxxopts      |    133 |        3393.31 |        1856.24 |
| args         |     54 |        1338.56 |        1078.00 |
| argparse     |     87 |        3251.03 |        2038.52 |

`ArgLite` performs the best, with total memory allocation being less than the peak memory of other libraries, and with fewer allocations. When passing a small number of arguments, ArgLite's advantage is even more pronounced. However, this advantage is also the least significant in this scenario, as even the highest consumption is far less than the C++ runtime's consumption. Since both versions use the same interface for getting positional arguments, their memory usage is almost identical. The difference between them lies in getting valued options, and such options are not used in large numbers in practice. Therefore, in extreme cases, the runtime performance of the Full version is almost the same as the Minimal version. The main difference is that the Full version has a longer compilation time and a larger binary size.

`args` also performs well, although its performance in the previous section was mediocre. As a heavy library, CLI11 can shorten compilation time by linking pre-built library files, but its runtime resource consumption is characteristic of a heavy library. `argparse`, which performed well in the previous section, has a mediocre performance in this section. As the number of passed arguments increases, its memory consumption grows faster, making it the most memory-intensive when 1002 positional arguments are passed. `cxxopts`, which claims to be a lightweight library, performs worse than CLI11. And as mentioned before, it defaults to including heavy libraries like `<regex>`, making it not lightweight at all. Not only does it perform poorly in terms of lightness, but it is also the only library among those tested that does not support subcommands.

## Summary

Since runtime performance is not very significant for practical applications, this summary is mainly based on compile-time performance.

ArgLite is indeed the best in terms of lightness, both at compile-time and run-time, and it provides an easy-to-use API and beautiful output. `argparse` also does well in terms of lightness and ease of use, but its output is not as friendly. CLI11 also has an easy-to-use API, but it is an all-in-one library, which is too heavy for simple and medium-sized programs. The other two libraries do not perform well in terms of either lightness or API ease of use, falling short of the others. `cxxopts` in particular, still performs mediocrely even after disabling `<regex>`, and its results would be much worse without this flag.

Of course, ease of use and aesthetics are relatively subjective metrics; only lightness can be measured with objective metrics. I have provided the test files, so you can easily compare which one is more user-friendly. Except for CLI11, the other libraries are also very easy to install, so you can easily compile the test files and compare their output messages. If you need formatted output, only ArgLite provides good support, `argparse` has very basic support, and the other libraries have none.

The biggest drawback of ArgLite is that it is a relatively new library. It has only passed its own project tests and has not been sufficiently tested in real-world use, making it unsuitable for serious projects. Therefore, the Full version is only to demonstrate the library's potential for medium-complexity projects, but it is not yet appropriate for such use. The Minimal version is intended for simple programs, for which its current reliability is sufficient.
