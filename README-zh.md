# 概述

已经有很多命令行参数解析库了，为什么要另外写一个？因为我想要一个轻量并且易用的库，但没找到满意的。

- [CLI11](https://github.com/CLIUtils/CLI11) 功能全面，接口易用，但太重了。
- [cxxopts](https://github.com/jarro2783/cxxopts) 号称轻量但其实也挺重的，而且接口不好用。
- [args](https://github.com/Taywee/args) 更轻量，但接口不好用。
- [argh](https://github.com/adishavit/argh) 足够轻量但太简陋了，连自动生成帮助都没有，所以也不好用。
- [argparse](https://github.com/morrisfranken/argparse) 足够轻量，接口易用，实现的方式很巧妙，是我最满意的库。不过帮助的格式比较特殊，不支持一些语法，例如 `-n123`。

所以我就写一个真正轻量、易用、功能足够丰富的库，并且帮助和错误信息更美观、更现代。关于它有多轻量，请查阅[跑分](#跑分)章节。关于易用，指按照一般的思路使用的话易于使用。具体体现为：

1. 使用命名清晰的函数添加和获取参数，不是使用重载运算符。
2. 一个语句就能获取一个参数。不需要先声明变量再绑定参数，或是先注册再取值，增删一个参数要修改多个位置的代码。

如果你需要这些不常规的用法的话请选择其他库：多线程解析命令行参数、先解析位置参数再解析选项。

本库有两个版本，一个是精简版，一个是完整版，分别 `#include "ArgLite/Minimal.hpp"`, `#include "ArgLite/Core.hpp"` 就能使用对应的版本。因为是头文件库，所以 include 后就能正常使用。

因为一开始就不追求全能，所以采用受限但轻量的特殊思路: 预处理后在注册单个选项时就返回解析结果，而不是先注册全部选项然后再解析。后来发现这种思路其实能实现很多高级功能，只是要引入更复杂的设计。所以分成两个版本，精简版仍保持极致的轻量，支持基本功能；完整版支持许多高级功能，比精简版复杂，但仍比大部分库轻量。当然完整版的功能还是比不上 CLI11 这种全能库，定位还是轻量库，只是支持的功能更多。

精简版支持以下功能

- 位置参数: `arg1 arg2 ...`
- 标志选项: `-v`, `--verbose`
- 带值选项: `-f path`, `--file=path`, `-fpath`
- 短选项组合: `-abc` (等同于 `-a -b -c`)
- 短选项组合带值: `-abcf file` (等同于 `-a -b -c -f file`)
- 所以对于标志参数 `-v, --verbose`, `-a` 和带值的选项 `-f, --file`，以下写法的效果一样
  - `--verbose -a --file path`
  - `-va --file=path`
  - `-vaf path`
  - `-vafpath`
- 灵活的位置参数: 位置参数不一定要在末尾，不在带值选项后面就行。以下写法的效果一样
  - `-v -n 123 file1 file2`
  - `-v file1 -n 123 file2`
- 互斥选项: `--enable-feature`, `--disable-feature`
- 选项结束标记: `--` 后面的参数都当成位置参数
- 解析 `bool`, `long long`, `double`, `std::string`
- 设置带值选项和位置参数的默认值
- 自动生成帮助 (`-h, --help`)
- 在帮助信息中给选项分组
- 格式化打印帮助和报错信息（例如彩色、加粗）

完整版支持

- 全部精简版的功能
- 子命令
- 标志计数: `-vvv`
- 多值选项: `-f file1 -f file2`, `-f file1,file2`
- 解析各种基本类型, 用 `std::optional` 区分有没有传值
- 修改帮助中选项的值的名称
- 将选项设置为必须提供

完整版的很多额外的功能都是通过链式调用实现，相关接口请查阅[这里](#optvalbuildert)。

完整版虽然功能更多，但对比其他现有的同类库优势不大，毕竟需要复杂功能的程序通常不要求命令行解析库能做得非常轻量。所以如果你正在考虑要不要使用这个库，我推荐你重点关注精简版，它是能提供基本参数解析和帮助生成的库中最轻量的（起码我没找到更轻量的），生成的帮助比很多更复杂的库美观。它非常适合用于需要基础命令行参数解析功能的小型程序，以很低的成本获取易用的命令行解析功能。确实有一些更轻量的库（例如上面提到的 argh），但它们的功能非常简陋，你可能需要写更多的和命令行参数解析相关的代码。

# 用法

首先介绍精简版的用法。两个版本的接口相似度很高，可以轻松迁移。

```cpp
#include "ArgLite/Minimal.hpp"

using namespace std;
using ArgLite::Parser;

int main(int argc, char **argv) {
    // 步骤1: 预处理，都是写这句
    Parser::preprocess(argc, argv);

    // 步骤2: 获取命令行参数
    auto verbose    = Parser::hasFlag("v,verbose", "Enable verbose output."); // 同时设置长短选项
    auto number     = Parser::getInt("number", "Number of iterations."); // 仅设置长选项
    auto rate       = Parser::getDouble("r", "Rate.", 123.0); // 仅设置短选项并设置默认值
    auto outputPath = Parser::getString("o,out-path", "Output file Path.", ".");
    auto outputFile = Parser::getPositional("output-file", "The output file name.");
    auto inputFiles = Parser::getRemainingPositionals("input-files", "The input files to process.");

    // 步骤3: 后处理，没有特殊需求的话直接调用这个函数就行了
    Parser::runAllPostprocess();

    cout << "Verbose    : " << boolalpha << verbose << '\n';
    cout << "Number     : " << number << '\n';
    cout << "Rate       : " << rate << '\n';
    cout << "Output Path: " << outputPath << '\n';
    cout << "Output file: " << outputFile << '\n';
    cout << "Input files:" << '\n';
    for (const auto &it : inputFiles) { cout << it << '\n'; }

    return 0;
}
```

```
> ./app --help
Usage: app [OPTIONS] output-file input-files

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
```

可见接口的命名已经足够清晰，使用方法也不难理解。先设置程序的描述和预处理，然后调用对应接口获取值，用变量保存返回值，最后后处理即可。这时你已经用变量获取到了所需的值，之后使用你自己定义的变量即可。

本库和其他同类库最大的区别是注册选项的同时就能获取值。上面的 `hasFlag` 和 `getXxx` 和其他库的 `add_option` 类似，都是提供参数的相关信息。

获取带值选项的函数的第一个参数都是选项名，第二个都是选项的描述，第三个是可选的默认值。选项名可以是短选项名或长选项名（不需要加上 `-` 或 `--`），也可以是两个都有（短选项名在前，长选项名在后，用 `,` 分隔）。如果不设置默认值的话，返回该类型的默认值（例如整数是 0）。

虽然上面说的是变量，但实际上用常量保存返回值也是可以的，因为返回的是一个右值。这也是本库的一个特点，其他库通常不能直接用常量保存解析结果，要先用变量保存解析结果然后用它初始化常量才能使用常量保存结果。

```cpp
    const auto number = Parser::getInt("number", "Number of iterations.");
```

和主流库相比，本库的一大优势是可以做到一条语句完成一个参数，多加一个参数只要多加一条语句。 CLI11 加一个参数要加一条声明语句，提供用于后续绑定的变量，然后添加一条注册语句。 cxxopts 则是要一条注册语句，解析后要一条取值语句。其他库大部分都是这两种情况，都有同一个参数的代码不够集中的问题，只是具体的操作不同。 CLI11 这种模式在直接用变量保存结果的情景下比本库更啰嗦，不过在使用对象保存结果的情景下表现得很好，因为对象的定义中总是要先声明成员。所以对于复杂程序来说，CLI11 的接口很好用，对于简单程序来说，本库的接口更好用。

完整版的用法类似，只是接口更多。对于上面用到的基本接口，区别在于 `getType()` 这种形式的函数换成 `get<T>()`，支持更多类型和更多功能。查阅[这里](#获取带值选项)了解详情。

## 流程

许多接口的调用顺序有一定要求，错误的顺序可能造成意外的结果。下面是一种合法的并且容易理解的顺序，如果你不想深究具体哪些接口有怎样的顺序要求的话，建议按照例子中的顺序使用。这个例子用的是完整版，精简版同理，只是没有子命令的接口，获取带值选项的接口不同。

**基本流程**

```markdown
- 程序信息（可选）
- 子命令（可选）
- 提供带值短选项（可选）
- 预处理
- 获取命令行选项
  - 获取标志选项或带值选项
  - 获取位置参数
  - （其实就是一般命令行参数的传递顺序，即各个选项间没有顺序要求，位置参数在最后）
- 后处理
```

**代码示例**

```cpp
using ArgLite::Parser;
using ArgLite::SubParser;

// === 程序信息（可选）
Parser::setDescription("A simple program to demonstrate ArgLite subcommand feature.");
Parser::setVersion("1.2.3");

// === 子命令（可选）
SubParser status("status", "Show the working tree status");
SubParser commit("commit", "Record changes to the repository");

// === 提供带值短选项（可选）
Parser::setShortNonFlagOptsStr("i");
commit.setShortNonFlagOptsStr("mF");

// === 预处理
Parser::preprocess(argc, argv);

// === 获取命令行选项
auto verbose    = Parser::countFlag("v,verbose", "Enable verbose output.");
auto indent     = Parser::get<int>("i,indent", "Option Description indent.").setDefault(26).setTypeName("num").get();
// 获取全部选项后才能获取位置参数，并且有多个位置参数时要按在命令行中的顺序写。
auto outputFile = Parser::getPositional("output-file", "The output file name.");
// 获取全部的单个位置参数后才能获取剩余的位置参数
auto inputFiles = Parser::getRemainingPositionals("input-files", "The input files to process.");

// 主命令和子命令各自内部保持上述顺序就行，之间的顺序没有要求，不一定是主命令在前。
// 因为这些函数不会影响到其他命令的数据。
auto commitAll      = commit.hasFlag("a,all", "Commit all changes.");
auto commitMsg      = commit.get<string>("m,message", "Use the given <msg> as the commit message.").get();
auto commitDate     = commit.get<int>("date", "Override the author date used in the commit.").get();
auto commitPathSpec = commit.getRemainingPositionals("pathspec", " When pathspec is given on the command line, ...", false);

// === 后处理
Parser::changeDescriptionIndent(indent); // optional
Parser::runAllPostprocess(); // 有更精细的控制需求的话，可以按照下方的接口说明手动逐个运行
```

可见其实按照一般人的思路调用就行了。其他同类库对顺序更宽容，但给的例子大致上也是这样的流程。文档中强调顺序是因为使用错误的顺序会得到错误的结果。如果你使用其他库时不会尝试非主流的顺序的话，不用太在意顺序问题。如果你还是觉得顺序不熟悉的话，把 `preprocess()` 当成其他库的 `parse()`，各种 get 函数是真的用于获取值而不是注册参数信息，注册只是顺便干的事。子命令则是实打实的注册，要在解析前面。所以还是常见的 `注册-解析-取值`。一般人也要注意的是可选的函数 `setShortNonFlagOptsStr()` 的顺序，详见[这里](#预处理)。

本库设计成解析时返回解析结果，由用户自己管理解析结果，完成解析后不需要再使用这个库。库内部的中间数据会被清除，降低运行时开销。所以如果你不使用子命令功能的话，运行 `Parser::runAllPostprocess();` 后你就不应该再次使用这个库的接口了。如果使用子命令的话，之后应该只使用 `Parser::isMainCmdActive()` 和 `.isActive()`。如果你继续使用各种获取参数的接口，只能获取到错误的数据。

本库也不支持多线程（应该没人用多线程解析命令行参数吧）。本库只支持最常见的用法：单线程环境单次解析，并且先写选项再写位置参数。

## 美化打印

```cpp
#define ARGLITE_ENABLE_FORMATTER
```

加上这个宏就能启用美化打印的功能。库会在输出到终端时，加上 ANSI 格式序列，给文本加上颜色、粗体或下划线。如果输出到管道或文件，则不会输出这些序列。

几乎没有 C++ 命令行参数解析库会考虑这个功能，导致 C++ 写的 CLI 程序的帮助不够美观、不够现代。实际上这个功能的基础版很容易实现，本库的精简版都能够支持。

效果如图所示：

![](https://raw.githubusercontent.com/InMirrors/images/main/ArgLite/formatter-help.png)
![](https://raw.githubusercontent.com/InMirrors/images/main/ArgLite/formatter-error.png)

## 接口说明

所有接口都在 `ArgLite::Parser` 类中以静态函数的形式提供。完整版还提供了 `ArgLite::SubParser` 类用于子命令。某个版本才有的接口会有提示，没有提示的话表明是两个版本都有的接口，并且用法一样，只是内部实现可能不同。

所有 `optName` 都是短选项名或长选项名（不需要加上 `-` 或 `--`），也可以是两个都有（短选项名在前，长选项名在后，用 `,` 分隔），例如 `o`, `--output`, `o,output`. `description` 都是用于在帮助中显示。

### 程序信息

这部分的函数其实不一定要在开头调用，在[后处理](#后处理)前调用就行了。不过一般实践都是开头就设置程序信息。

```cpp
void setDescription(std::string description);
```

设置程序描述，在帮助信息的第一行显示。不调用的话只是帮助中没有这行而已，不影响其他功能。

---

```cpp
void setVersion(std::string versionStr);
```

设置程序版本，并添加 `-V` 和 `--version` 选项用于打印版本信息。如果不想 `-V` 被库占用的话，可以不调用这个函数，自己处理版本打印。

### 预处理

```cpp
void setShortNonFlagOptsStr(std::string shortNonFlagOptsStr);
```

不调用这个函数也没关系，只是支持的命令行参数语法会少一种。如果你刚接触这个库，建议先不使用它。

因为在预处理的时候，库并不知道选项的具体信息，当遇到 `-n123` 这种参数时，不知道是 `-n -1 -2 -3`, `-n 123`, `-n -1 23` 还是 `-n -1 -2 3`。默认是把它们都当成短标志选项，即 `-n -1 -2 -3`。如果你想支持带值短选项后面马上接它的值的话，你必须在预处理前告诉库，即调用 `setShortNonFlagOptsStr()`，它的参数是以字符串的形式书写的所有带值短选项。这样预处理时才能正确识别带值短选项和值。注意只是传入*带值选短选项*，不是所有的短选项，用作标志的短选项不需要也不能传入。

这个缺点是这个库最不优雅的一点，这种独特的处理思路很难优雅地提供这个语法。好在不影响核心功能。

为了减少在添加带值短选项时出现遗漏的可能性，建议使用下面的 RegEx 查找所有的带值短选项。在支持多光标的编辑器 / IDE 中通常可以用它选中所有带值短选项。

```js
// 用于查找主命令的带值短选项
(?<=::get.+?\(").(?=[,"])
// 用于查找子命令的带值短选项, `subcmd` 要替换成实际的子命令变量名
(?<=subcmd\.get.+?\(").(?=[,"])
```

---

```cpp
void preprocess(int argc, const char *const *argv);
```

预处理命令行参数，这是使用本库的第一步。你可能注意到前面给的[流程](#流程)中 `preprocess()` 并不是第一步。因为前面的都是可选的，这个是必须运行的函数中最前面的。

程序信息其实在后处理前提供就行，写在最前面只是为了符合一般同类库的实践。

真正要在 `preprocess()` 前面的是 `setShortNonFlagOptsStr()` 和[子命令](#子命令)。

### 获取标志选项

```cpp
bool hasFlag(std::string_view optName, std::string description);
```

检查是否存在标志选项，存在一个或多个时返回 `true`。

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

检查两个互斥选项是否存在。如果前者存在后者不存在，返回 `true`；如果后者存在前者不存在，返回 `false`；如果两者都不存在，返回 `defaultValue`；如果两者都存在，则后出现的选项生效。因为传入的是一个结构体，传入参数时要加上 `{}`。在 C++20 及以上，你可以使用指定初始化器 (Designated initializers) 增强代码的可读性：

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

**完整版独有。** 计算标志选项出现的次数，不管是长选项还是短选项都会使计数器 +1。

### 获取带值选项

```cpp
// 精简版
// getInt, getDouble, getString, getBool 的签名都差不多，只是具体的类型不同
type getType(std::string_view optName, const std::string &description, type defaultValue = type{});

// 完整版
template <typename T>
OptValBuilder<T> get(std::string_view optName, std::string description);
```

精简版直接返回指定类型的值，整数和浮点数都只返回一种，需要其他类型的话，需要手动转换类型。例如需要 `unsigned` 的话需要自己从返回的 `long long` 转换。如果需要的类型会溢出的话，例如需要 `unsigned long long`，需要改用完整版，或者用 `getString()` 获取字符串后自己解析。

完整版返回一个 `OptValBuilder<T>` 对象，支持几乎全部的内置类型，可以通过链式调用配置更多功能，最后调用 `get()` 或 `getVec()` 获取值。具体支持的类型请查阅源代码中的 [`convertType()`](./include/ArgLite/GetTemplate.hpp#L9-L44)

不管哪个版本，获取 `bool` 类型时，都是不区分大小写，除以下参数外的参数都是错误参数。

- 真: `1`, `true`, `yes`, `on`
- 假: `0`, `false`, `no`, `off`

#### `OptValBuilder<T>`

**完整版独有。** `get<T>()` 返回的中间对象，提供以下方法：

- `setDefault(T defaultValue)`: 设置默认值。

- `setTypeName(std::string typeName)`: 在帮助信息中设置值的类型名称。

  例如默认会显示 `--file <string>`, `setTypeName("path")` 后变成 `--file <path>`。

- `required()`: 将选项设置为必需，运行程序而不提供这个选项的话会报错。

- `T get()`: 获取单个值。

- `std::vector<T> getVec(char delimiter = '\0')`: 获取多个值组成的 `vector`。

  直接调用的话，`-f file1 -f file2` 得到 `[file1, file2]`

  如果提供了分隔符，每个值都会根据分隔符进一步分割成多个值。`getVec(',')` 后传入 `-f file1 -f file2,file3` 会得到 `[file1, file2, file3]`

### 获取位置参数

```cpp
std::string getPositional(
    const std::string &posName, std::string description, bool required = true，
    std::string defaultValue = "");
std::vector<std::string> getRemainingPositionals(
    const std::string &posName, std::string description, bool required = true，
    const std::vector<std::string> &defaultValue = {});
```

本库提供获取一个位置参数和获取剩余全部位置参数的接口。必须在所有调用完全部和选项相关的接口（就是上面两个小节的接口）之后调用。各个位置参数的添加像其他库那样，先添加的消耗前面的命令行参数。一些库可以在获取多个位置参数后，再获取单个位置参数，实现这样的用法：

```bash
command input1 input2... output
```

本库不支持这种用法。因为这个库会即时返回解析结果，调用时会消耗掉一个位置参数，调用 `getRemainingPositionals()` 会消耗全部的位置参数。要获取多个位置参数的话只能写成这样：

```bash
command output input1 input2...
```

两个接口都有一个可选参数 `required`，只有后面调用的函数才能设置为 `false`。类似于 C++ 中的函数默认参数，只有后面的才能可选，并且不能跳过中间的可选参数写后面的可选参数。可选并且没提供命令行参数的话，返回 `defaultValue`

因为绝大部分应用的位置参数都是字符串类型，所以为了轻量和易用，本库只支持以字符串形式解析位置参数。如果你需要解析成其他类型，请尝试改成带值选项或使用其他库。如果非要用这个库并且用位置参数形式提供，你可以获取字符串后手动转换，或者修改源代码，暴露 `ArgLite::Parser::convertType<T>()`, 它支持 `bool`, `char`, `std::optional` 这几种标准库不支持转换的类型。

### 后处理

```cpp
void changeDescriptionIndent(size_t indent);
```

更改帮助信息中选项描述的缩进，默认是 25。它用于调整帮助信息的显示，一般不需要使用它。

例如程序的选项名部分很多都是刚好超长了一点，超长的选项的描述会在下一行显示，用这个函数把缩进调大点，就能让这些选项的描述在同一行显示，更加美观。其实[用法](#用法)中的简单示例就属于这种情况，调成 27 后就能全部在同一行显示

和下面的后处理接口不同，这个其实不一定要在后处理阶段调用，在 `tryToPrintHelp()` 前调用就行，你甚至可以在一开始就调用。只是在这里调用的话更容易理解使用流程。

---

```cpp
void tryToPrintHelp();
```

如果用户提供了 `-h` 或 `--help`，则打印帮助信息并退出程序。

```cpp
bool tryToPrintInvalidOpts(bool notExit = false);
```

检查并报告所有未知的选项。

```cpp
bool finalize(bool notExit = false);
```

完成解析，打印所有累积的错误信息。例如带值选项缺少参数、带值选项参数类型错误、缺少位置参数。运行这个函数后，除了使用[子命令](#子命令)功能时可以使用检查当前激活命令的接口外，任何本库的接口都不要再使用。这时库已经将所有解析结果返回，你只需使用之前保存的结果。

```cpp
bool runAllPostprocess(bool notExit = false);
```

依次运行 `tryToPrintHelp()`, `tryToPrintInvalidOpts()` 和 `finalize()`。

一般情况下直接运行 `runAllPostprocess()` 就行。后面几个函数都会在出错时直接退出程序。如果你不想直接退出程序的话，传入 `true`，函数会在发生错误时返回 `true`。如果你需要更精细的控制，例如有未知选项时不退出，但解析错误时退出，可以依次运行 `tryToPrintHelp(); auto hasInvalidOpts = tryToPrintInvalidOpts(true); finalize();`。

可见本库不会抛出异常，用返回值判断是否出现错误。因为这种轻量库会出现错误的地方基本只有传入错误的命令行参数，需要退出程序让用户重新输入。这种场景下返回值已经足够，并且契合轻量的定位。

### 选项分组

```cpp
void insertOptHeader(std::string header);
```

本库像其他库那样，帮助信息中的选项按注册顺序排列，注册一个选项就会在帮助信息中插入一个条目。添加选项分组的思路也是如此，用这个函数插入一个分组标题。打印帮助信息的函数遇到分组标题时，会自动加上换行符和 `:`，保持和默认标题相同的风格。调用这个函数后，原本的 `Options:` 标题会消失，所以你要自己补上你想要的标题。

以下是一个模仿 [ripgrep](https://github.com/BurntSushi/ripgrep) 的帮助，你可以在 examples 文件夹中找到[完整代码](./examples/option_grouping.cpp)。

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

它会得到这样的帮助信息：

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

如果不加任何的 `insertOptHeader()` 语句，帮助信息是这样的：

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

你可能会觉得这种实现太粗糙了，根本没有把选项打包到各个组内，只是简单地插入一个帮助条目。实际上其他库的分组一般也只是影响帮助中的显示，各个选项的存储和使用和不分组时没有本质区别，不是按照类似于 `args["group"]["opt"]` 的方式取值。本库定位轻量，能以简单的方式实现就不会用复杂的方式，虽然这种方式比较另类。

### 子命令

**完整版独有。**通过创建 `SubParser` 对象来定义子命令。

本库有这条规则：“**静态方法用于主命令，成员方法用于子命令**”。所以不使用子命令功能时，你不会创建任何对象，使用子命令功能时也只会创建子命令对象。绝大部分同类库都会创建一个对象用于解析命令行参数，而本库为了轻量和易用，采用以静态为主的设计思路。毕竟一般都是只解析一份命令行参数，不会多次解析，所以静态方法和静态数据就能满足。

本库只支持一级子命令，不支持多级子命令。需要多级子命令的程序应该也不会要求命令行参数解析库能做得很轻量，不是本库的目标用户。

为什么只支持一级子命令？类本身就有层级结构，静态的是底层，成员的是上一层，并且前者只有一份，后者可以有多份。这个结构正好是一级子命令的样子，所以可以利用这个特点轻松地实现一级子命令。`Parser` 和 `SubParser` 对象本身没有任何的层级结构，是借助语言特性才能以低成本形成二级树形结构，要实现更高层级的子命令的话会让库显著变重。因此现在没有多级子命令功能，以后大概率也不会有。

具体用法参考这个[示例](examples/subcommand.cpp)，这个示例也演示了完整版的绝大部分功能。

---

```cpp
SubParser(std::string subCommandName, std::string subCmdDescription);
```

构造函数。`subCommandName` 是子命令的名称，`subCmdDescription` 是子命令描述。这两者都会在帮助信息中显示。

---

```cpp
bool isActive();
```

检查该子命令是否被激活。因为注册选项时就会返回一个值，不管当前激活的命令是哪个，没激活时会返回类型的默认值。所以提供这个成员方法用于检查该命令是否激活，还提供 `bool Parser::isMainCmdActive()` 用于检查主命令是否激活。

---

`SubParser` 对象也拥有 `setShortNonFlagOptsStr`, `hasFlag`, `countFlag`, `hasMutualExFlag`, `get`, `getPositional`, `getRemainingPositionals` 等方法，用法与 `Parser` 中的版本相同，但只在该子命令激活时生效。

# 跑分

todo

# 其他特性

不像绝大部分库，添加选项时还不能获取值，要等解析时才统一赋值，本库添加选项和获取值都由同一个函数完成，添加选项时就已经获取值。所以可以在拿到值后，将这个值作为参数传递给库的接口，控制库的行为。即可以在运行时调整解析行为。

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
    for (const auto &it : include) { cout << it << '\n'; }

    return 0;
}
```

这个例子可以用 `-i, --indent` 控制帮助信息的缩进。

```pwsh
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

这个用法似乎什么实际意义，只是演示上述特性。接下来看一个比较实用的例子——改变多值选项的分隔符。

```pwsh
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

如果传入的参数不能用一个固定的分隔符应对所有情况，可以利用这个特性，动态控制分隔符，选取适合当前参数的分隔符。
