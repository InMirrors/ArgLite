- [概述](#概述)
  - [精简版特性](#精简版特性)
  - [完整版特性](#完整版特性)
- [用法](#用法)
  - [流程](#流程)
  - [美化打印](#美化打印)
  - [接口说明](#接口说明)
    - [程序信息](#程序信息)
    - [预处理](#预处理)
    - [获取标志选项](#获取标志选项)
    - [获取带值选项](#获取带值选项)
      - [`OptValBuilder<T>`](#optvalbuildert)
      - [解析自定义类型](#解析自定义类型)
    - [获取位置参数](#获取位置参数)
    - [后处理](#后处理)
      - [打印帮助](#打印帮助)
      - [处理错误](#处理错误)
    - [选项分组](#选项分组)
    - [子命令](#子命令)
  - [其他特性](#其他特性)
- [跑分](#跑分)
  - [编译耗时和二进制体积](#编译耗时和二进制体积)
  - [运行时内存消耗](#运行时内存消耗)
  - [总结](#总结)

# 概述

已经有很多命令行参数解析库了，为什么要另外写一个？因为我想要一个轻量并且易用的库，但没找到满意的。

- [CLI11](https://github.com/CLIUtils/CLI11) 功能全面，接口易用，但太重了。
- [cxxopts](https://github.com/jarro2783/cxxopts) 号称轻量但其实也挺重的，而且接口不好用。
- [args](https://github.com/Taywee/args) 非常轻量，但接口不好用。
- [argh](https://github.com/adishavit/argh) 足够轻量但太简陋了，连自动生成帮助都没有，所以也不好用。
- [argparse](https://github.com/morrisfranken/argparse) 足够轻量，接口易用，实现的方式很巧妙，是我最满意的库。不过帮助的格式比较特殊，不支持一些语法，例如 `-n123`。

所以我就写一个真正轻量、易用、功能足够丰富的库，并且帮助和错误信息更美观、更现代。关于它有多轻量，请查阅[跑分](#跑分)章节。关于易用，指按照一般的思路使用的话易于使用。具体体现为：

1. 使用命名清晰的函数添加和获取参数，不是使用重载运算符。
2. 一个语句就能获取一个参数。不像大部分库那样需要先声明变量再绑定参数，或是先注册再取值，它们增删一个参数要修改多个位置的代码。

如果你需要这些不常规的用法的话请选择其他库:

1. 多线程解析命令行参数。
2. 给同一个选项添加多个短选项或多个长选项，例如 `-f, -i, --file, --input` 都属于同一个选项。
3. 先解析位置参数再解析选项。

本库有两个版本：**精简版**和**完整版**。分别 `#include "ArgLite/Minimal.hpp"`, `#include "ArgLite/Core.hpp"` 就能使用对应的版本。因为是头文件库，所以 include 后就能正常使用。

精简版仍保持极致的轻量，支持基本功能；完整版支持许多高级功能，比精简版复杂，但仍比大部分库轻量。当然完整版的功能还是比不上 CLI11 这种全能库，定位还是轻量库，只是支持的功能更多。本库在一定程度上符合 C++ 的 "You don't pay for what you don't use" 理念。

- **精简版**以很低的开销实现基础功能和易用的接口，适合只需要基本的命令行参数解析的情况。
- **完整版**以很低的开销扩展了核心的高级功能，适合需要构建具有一定复杂度的程序的情况。

## 精简版特性

- 位置参数: `arg1 arg2 ...`
- 标志选项: `-v`, `--verbose`
- 带值选项: `-f path`, `--file=path`, `-fpath`
- 短选项组合: `-abc` (等同于 `-a -b -c`)
- 短选项组合带值: `-abcf file` (等同于 `-a -b -c -f file`)
- 所以对于标志选项 `-v, --verbose`, `-a` 和带值选项 `-f, --file`，以下写法的效果一样
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
- **格式化输出**: 给帮助和报错信息添加彩色和加粗

## 完整版特性

- 全部精简版的功能
- 子命令
- 标志计数: `-vvv`
- 多值选项: `-f file1 -f file2`, `-f file1,file2`
- 解析各种基本类型, 用 `std::optional` 区分有没有传值
- 自定义帮助中选项的值的名称
- 将选项设置为必须提供

完整版的很多额外的功能都是通过链式调用实现，相关接口请查阅[这里](#optvalbuildert)。

完整版虽然功能更多，但对比其他现有的同类库优势不大，毕竟需要复杂功能的程序通常不要求命令行解析库能做得非常轻量。所以如果你正在考虑要不要使用这个库，我推荐你重点关注精简版，它是能提供基本参数解析和帮助生成的库中最轻量的（起码我没找到更轻量的），生成的帮助比很多更复杂的库美观。它非常适合用于需要基础命令行参数解析功能的小型程序，以很低的成本获取易用的命令行解析功能。确实有一些更轻量的库（例如上面提到的 argh），但它们的功能非常简陋，你可能需要写更多的和命令行参数解析相关的代码。

# 用法

这里介绍精简版的用法。两个版本的接口相似度很高，可以轻松迁移。

```cpp
#include "ArgLite/Minimal.hpp"

using namespace std;
using ArgLite::Parser;

int main(int argc, char **argv) {
    // 步骤1: 预处理
    Parser::preprocess(argc, argv);

    // 步骤2: 获取命令行参数
    auto verbose    = Parser::hasFlag("v,verbose", "Enable verbose output."); // 同时设置长短选项
    auto number     = Parser::getInt("number", "Number of iterations."); // 仅设置长选项
    auto rate       = Parser::getDouble("r", "Rate.", 123.0); // 仅设置短选项并设置默认值
    auto outputPath = Parser::getString("o,out-path", "Output file Path.", ".");
    auto outputFile = Parser::getPositional("output-file", "The output file name.");
    auto inputFiles = Parser::getRemainingPositionals("input-files", "The input files to process.");

    // 步骤3: 后处理
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

程序输出：

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

可见接口的命名已经足够清晰，使用方法也不难理解：先设置程序的描述和预处理，然后调用对应接口获取值，用变量保存返回值，最后后处理即可。这时你已经用变量获取到了所需的值，之后使用你自己定义的变量即可。

**和其他同类库最大的区别**: 不像其他库需要先注册完全部选项后才能解析然后取值，本库注册选项的同时就能获取值。上面的 `hasFlag` 和 `getXxx` 和其他库的 `add_option` 类似，都是提供参数的相关信息，但它们会同时返回解析结果。

这些接口的具体用法请查阅[接口说明](#接口说明)章节。

虽然上面的例子用变量保存了返回值，但实际上用常量保存返回值也是可以的，因为返回的是一个右值。这也是本库的一个特点，其他库通常不能直接用常量保存解析结果，要先用变量保存解析结果然后用它初始化常量。

```cpp
// 你可以用常量保存结果
const auto number = Parser::getInt("number", "Number of iterations.");
```

和主流库相比，本库的一大优势是可以做到一条语句完成一个参数，多加一个参数只要多加一条语句。 CLI11 加一个参数要加一条声明语句，提供用于后续绑定的变量，然后添加一条注册语句。 cxxopts 则是要一条注册语句，解析后要一条取值语句。其他库大部分都是这两种情况，都有同一个参数的代码不够集中的问题，只是具体的操作不同。 CLI11 这种模式在直接用变量保存结果的情景下比本库更啰嗦，不过在使用对象保存结果的情景下表现得很好，因为对象的定义中总是要先声明成员。所以对于复杂程序来说，CLI11 的接口很好用，对于简单程序来说，本库的接口更好用。

**完整版**的用法类似，只是接口更多。对于上面用到的基本接口，区别在于 `getType()` 这种形式的函数换成 `get<T>()`，支持更多类型和更多功能。查阅[这里](#获取带值选项)了解详情。

## 流程

许多接口的调用顺序有一定要求，错误的顺序可能造成意外的结果。下面是一种合法的并且容易理解的顺序，如果你不想深究具体哪些接口有怎样的顺序要求的话，建议按照例子中的顺序使用。这个例子用的是完整版，精简版同理，只是没有子命令的接口，获取带值选项的接口不同。

**基本流程**

```markdown
- 程序信息（可选）
- 子命令（可选）
- 提供带值短选项（可选）
- **预处理**
- **获取命令行选项** (先标志选项和带值选项, 然后位置参数)
- **后处理**
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

所有 `optName` 都是短选项名或长选项名（不需要加上 `-` 或 `--`），也可以是两个都有（短选项名在前，长选项名在后，用 `,` 分隔），例如 `o`, `output`, `o,output`. `description` 都是用于在帮助中显示。

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

检查两个互斥选项是否存在。返回值如下:
- 如果前者存在后者不存在，返回 `true`
- 如果后者存在前者不存在，返回 `false`；
- 如果两者都不存在，返回 `defaultValue`；
- 如果两者都存在，则后出现的选项生效。

因为传入的是一个结构体，传入参数时要加上 `{}`。在 C++20 及以上，你可以使用指定初始化器 (Designated initializers) 增强代码的可读性：

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
T getType(std::string_view optName, const std::string &description, T defaultValue = T{});

// 完整版
template <typename T>
OptValBuilder<T> get(std::string_view optName, std::string description);
```

- **精简版**直接返回指定类型的值，整数和浮点数都只返回一种，需要其他类型的话，需要手动转换类型。例如需要 `unsigned` 的话需要自己从返回的 `long long` 转换。如果需要的类型会溢出的话，例如需要 `unsigned long long`，需要改用完整版，或者用 `getString()` 获取字符串后自己解析。
- **完整版**返回一个 `OptValBuilder<T>` 对象，支持几乎全部的内置类型，可以通过链式调用配置更多功能，最后调用 `get()` 或 `getVec()` 获取值。具体支持的类型请查阅源代码中的 [`convertType()`](./include/ArgLite/GetTemplate.hpp#L9-L44)。除了基本类型，还支持 `std::optional`, 你可以用它包装其他支持的类型，以区分有没有通过命令行参数提供选项的值。因为用户没有传值或传了默认值时，获取普通类型会得到默认值。如果你想区分有没有传值，这正是你需要的。

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

#### 解析自定义类型

本库**不**提供对自定义类型的完整支持，只提供非常有限的支持。如果你需要完善的支持，请选择其他库。本库的核心理念是轻量，高级功能如果能以轻量的方式实现的话，本库乐于支持，否则不会支持。支持自定义类型不仅要处理解析和验证，还要能生成合适的帮助和错误信息，会明显增加库的重量，所以不会支持。

本节说明如何利用有限的支持处理自定义的帮助信息和错误信息。**仅限完整版**。

---

**设置错误信息**

对于未知类型, `get<T>()` 会尝试使用解析到的字符串去初始化它。如果你的自定义类型可以用字符串初始化，那么理论上可以直接用它获取解析结果。但它无法提供校验，帮助和错误信息中不能提供相关信息。所以更推荐下面这种用 `get<std::string>()` 的方法。

先获取参数的原始字符串，然后写你的解析和校验逻辑，在参数错误分支调用这个接口插入错误信息。

```cpp
void pushBackErrorMsg(std::string msg);
```

它会往内部的存储错误信息的数组中插入你提供的字符串，在后处理阶段使用。

例如[子命令示例](./examples/subcommand.cpp#L57-L70)中的 `grep` 子命令的 `--option` 选项接受 "auto, always, never", 这段代码校验这个选项的值：

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

传入错误的参数时会打印

```
>./subcommand grep --color blue
Errors occurred while parsing command-line arguments.
The following is a list of error messages:
Error: Invalid value for option '--color'. Expected 'auto', always' or 'never', but got 'blue'.
```

如果你需要像库的输出那样提供精细的格式化文本，你可以调用 `ArgLite::Formatter` 下的接口。总共有 `red()`, `yellow()`, `bold()`, `boldUnderline()` 这几个函数，它们的签名中只是函数名部分不一样。

```cpp
std::string red(std::string_view str, const std::ostream &os = std::cerr);
```

它们会返回一个格式化后的字符串，也是输出到文件或管道时不添加 ANSI 序列。第二个参数是用于检测的输出流，函数会根据提供的流判断它是否输出到终端。 `bold()`, `boldUnderline()` 的第二个参数的默认值是 `std::cout`, 但错误信息输出到 `std::cerr`, 所以在这里使用它们时需要给第二个参数传入 `std::cerr`. 你也可以统一给第二个参数传入 `std::cerr`, 这样就不需要记它们的默认值。

像[子命令示例](./examples/subcommand.cpp#L62-L70)那样写的话可以得到这样的输出：

![](https://raw.githubusercontent.com/InMirrors/images/main/ArgLite/formatter-custom-types.png)

你只要在后处理前调用 `pushBackErrorMsg` 就能在用户输入错误参数时打印错误信息。但如果你对错误信息的顺序有要求，需要在下一次调用获取命令行参数的接口前就调用，不然下一次调用的错误信息会排在前面。

---

**设置帮助信息**

因为库不识别自定义类型，所以如果你要在帮助信息中说明选项接受怎样的值的话，需要手动调整帮助信息。[子命令示例](./examples/subcommand.cpp#L57-L61)中的这段代码

```cpp
auto grepColor = grep.get<string>("color", "When to use colors. [possible values: auto, always,\n"
                                           "never].")
                     .setDefault("auto")
                     .setTypeName("when")
                     .get();
```

可以得到

```
      --color <when>      When to use colors. [possible values: auto, always,
                          never]. [default: auto]
```

或者在值的类型名中提示有效参数，描述中不提示：

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

### 获取位置参数

```cpp
std::string getPositional(
    const std::string &posName, std::string description, bool required = true，
    std::string defaultValue = "");
std::vector<std::string> getRemainingPositionals(
    const std::string &posName, std::string description, bool required = true，
    const std::vector<std::string> &defaultValue = {});
```

本库提供获取一个位置参数和获取剩余全部位置参数的接口。必须在所有调用完全部和选项相关的接口（就是上面两个小节的接口）**之后**调用。各个位置参数的添加像其他库那样，先添加的消耗前面的命令行参数。

一些库可以在获取多个位置参数后，再获取单个位置参数，实现这样的用法：

```bash
command input1 input2... output
```

本库**不**支持这种用法。因为这个库会即时返回解析结果，调用时会消耗掉一个位置参数，调用 `getRemainingPositionals()` 会消耗全部的位置参数。要获取多个位置参数的话只能写成这样：

```bash
command output input1 input2...
```

两个接口都有一个可选参数 `required`，只有后面调用的函数才能设置为 `false`。类似于 C++ 中的函数默认参数，只有后面的才能可选，并且不能跳过中间的可选参数写后面的可选参数。可选并且没提供命令行参数的话，返回 `defaultValue`

因为绝大部分应用的位置参数都是字符串类型，所以为了轻量和易用，本库只支持以字符串形式解析位置参数。如果你需要解析成其他类型，请尝试改成带值选项或使用其他库。如果非要用这个库并且用位置参数形式提供，你可以获取字符串后手动转换，或者修改源代码，暴露 `ArgLite::Parser::convertType<T>()`, 它支持 `bool`, `char`, `std::optional` 这几种标准库不支持转换的类型。

### 后处理

#### 打印帮助

```cpp
void changeDescriptionIndent(size_t indent);
```

更改帮助信息中选项描述的缩进，默认是 25。它用于调整帮助信息的显示，一般不需要使用它。

例如程序的选项名部分很多都是刚好超长了一点，超长的选项的描述会在下一行显示，用这个函数把缩进调大点，就能让这些选项的描述在同一行显示，更加美观。其实[用法](#用法)中的简单示例就属于这种情况，调成 27 后就能全部在同一行显示

---

```cpp
void setHelpFooter(std::string_view footer);
```

用于添加额外的帮助信息，在自动生成的帮助信息的后面打印传入的字符串。你可以使用这个接口添加使用示例、联系方式等信息。如果你需要像库自动生成的帮助那样提供精细的格式化文本，你可以调用 `ArgLite::Formatter` 下的接口。用法参考[解析自定义类型](#解析自定义类型)章节的说明。

例如[子命令示例](./examples/subcommand.cpp#L77-L83)中有这些代码：

```cpp
// Set the help footer
string footer;
footer += ArgLite::Formatter::boldUnderline("Examples:\n");
footer += "  subcommand -v out.txt in1.txt in2.txt\n";
footer += "  subcommand status\n";
footer += "  subcommand commit -m \"An awesome commit\"";
Parser::setHelpFooter(footer);
```

可以得到这样的输出：

```
...
  -V, --version           Show version information and exit
  -h, --help              Show this help message and exit

Examples:
  subcommand -v out.txt in1.txt in2.txt
  subcommand status
  subcommand commit -m "An awesome commit"
```

<details><summary>点击查看完整帮助信息截图</summary>

![](https://raw.githubusercontent.com/InMirrors/images/main/ArgLite/formatter-footer.png)

</details>

和下一小节的错误处理接口不同，`changeDescriptionIndent()` 和 `setHelpFooter()` 其实不一定要在后处理阶段调用，在 `tryToPrintHelp()` 前调用就行，你甚至可以在一开始就调用。只是在这里调用的话更容易理解使用流程。

---

```cpp
void tryToPrintHelp();
```

如果用户提供了 `-h` 或 `--help`，则打印帮助信息并退出程序。

因为选项名要加前缀 `-` 并且有长短选项，所以选项的描述缩进会比位置参数或子命令的描述大不少。这导致选项描述比较容易过长，超过终端的宽度，在下一行显示。终端并不会缩进显示下一行，导致描述也在行首的部分，而这部分本应是选项的区域。最终形成描述和选项混在一起的问题，增加分辨难度，影响用户使用体验。例如[子命令示例](./examples/subcommand.cpp)中的

```cpp
    auto commitSignOff  = commit.hasMutualExFlag({
        "s,signoff",
        "Add a Signed-off-by trailer by the committer at the end of the commit log message.",
        "no-signoff",
        "Do not add a Signed-off-by trailer by the committer at the end of the commit log message.",
        false,
    });
```

在帮助中的输出为

```
  -s, --signoff           Add a Signed-off-by trailer by the committer at theend of the commit log message.
      --no-signoff        Do not add a Signed-off-by trailer by the committerat the end of the commit log message. (default)
```

如果你在描述字符串中加上 `\n` 后，例如

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

打印帮助时会给后面的行添加缩进，使得描述的内容总是在右边区域，得到下面这样的输出

```
  -s, --signoff           Add a Signed-off-by trailer by the committer at the
                          end of the commit log message.
      --no-signoff        Do not add a Signed-off-by trailer by the committer
                          at the end of the commit log message. (default)
```

如果库能自动分割描述的话自然更好，但要足够智能地分割的话会引入很多代码，和轻量库的定位不符。更何况手动添加换行符并不复杂，还能让代码的显示效果更接近帮助输出的效果。所以本库采取这种不够智能的实现。

#### 处理错误

```cpp
bool tryToPrintInvalidOpts(bool notExit = false);
```

检查并报告所有未知的选项。

```cpp
bool finalize(bool notExit = false);
```

完成最后的工作，打印所有累积的错误信息。例如打印带值选项缺少参数、带值选项参数类型错误、缺少位置参数。这个函数会清除大量内部的中间数据，所以运行这个函数后，除了使用[子命令](#子命令)功能时可以使用检查当前激活命令的接口外，任何本库的接口都不要再使用。这时库已经将所有解析结果返回，你只需使用之前保存的结果。

```cpp
bool runAllPostprocess(bool notExit = false);
```

依次运行 `tryToPrintHelp()`, `tryToPrintInvalidOpts()` 和 `finalize()`。

一般情况下直接运行 `runAllPostprocess()` 就行。后面几个函数都会在出错时直接退出程序。如果你不想直接退出程序的话，传入 `true`，函数会在发生错误时返回 `true`。如果你需要更精细的控制，可以手动调用这些函数。例如有未知选项时不退出，但解析错误时退出，可以依次运行 `tryToPrintHelp(); auto hasInvalidOpts = tryToPrintInvalidOpts(true); finalize();`。

本库不会抛出异常，用返回值判断是否出现错误。因为这种轻量库会出现错误的地方基本只有传入错误的命令行参数，需要退出程序让用户重新输入。这种场景下返回值已经足够，并且契合轻量的定位。

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

## 其他特性

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
    for (const auto &it : include) { cout << "  " << it << '\n'; }

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

# 跑分

本环节测试的 argparse 库不是常见的 [p-ranav/argparse](https://github.com/p-ranav/argparse), 而是 [morrisfranken/argparse](https://github.com/morrisfranken/argparse)。前者的接口类似于 Python argparse, 也类似于 cxxopts, 都是用字符串查询解析结果。这种用法对于 Python 这类动态类型语言来说很方便，但对于 C++ 这种静态类型语言来说并不方便，需要写额外的代码才能获取结果，补全工具不能提供补全。所以这里只测试了同类的、知名度更高的 cxxopts. morrisfranken/argparse 只是名字一样，但外部接口和内部实现的差异巨大，提供了更易用的接口。

使用 cxxopts 时，只要编译器不是很老旧，它默认引入 `<regex>`，使得库变得非常重。它的简介是 "Lightweight C++ command line option parser", 但它的文档没说明会引入重型库。直接使用的话，编译耗时和二进制体积都显著比其他库高。所以测试时加上 `CXXOPTS_NO_REGEX`, 不然它的结果会很难看。

除 CLI11 外，其他库都是头文件库。CLI11 也有头文件库的版本，但用这个版本的话测试结果会很难看，所以使用常规的需要链接库文件的版本。也因此 CLI11 在集成方面的易用性比不上其他库。

测试内容是各个库实现最基础功能时的开销。测试的功能包括：获取标志选项、获取整数、获取一个位置参数、获取剩余位置参数。测试文件都位于 [benchmark](./tests/benchmark/)，你可以对比各个 cpp 文件的内容，了解各个库的接口易用性。对比后，我想大部分人都会认可这个结论: ArgLite, argparse, CLI11 都很易用, args 和 cxxopts 不但代码量更多，也没那么容易理解用法。

各个库都使用 2025-12-21 时的最新版，在 WSL 上进行测试。具体的结果会因为测试平台、测试环境、库的版本等因素的影响，数值可能和下面的结果有明显差异。但各个库之间的相对差异应该比较稳定，不会推翻结论。

## 编译耗时和二进制体积

除了 "ArgLite Sub", 其他条目都是实现了“获取标志选项、获取整数、获取一个位置参数、获取剩余位置参数”的简单程序，用于横向比较。 "ArgLite Sub" 是使用了几乎所有的完整版接口的[示例](./examples/subcommand.cpp)的结果，是用复杂度高很多的程序，用于纵向对比。 "ArgLite Full" 是完整版的简单程序的测试结果，并非完整示例。

使用编译参数 `-s -DNDEBUG` 得到的结果如下。

| Name          | Time (s) | Size (KB) | Time (%) | Size (%) |
| ------------- | -------: | --------: | -------: | -------: |
| ArgLite Mini  |     0.94 |     110.2 |   100.0% |   100.0% |
| ArgLite Full  |     1.08 |     134.2 |   114.9% |   121.8% |
| *ArgLite Sub* |     1.21 |     162.3 |   128.7% |   147.3% |
| CLI11         |     1.46 |     550.3 |   155.3% |   499.4% |
| cxxopts       |     1.98 |     278.3 |   210.6% |   252.5% |
| args          |     1.65 |     278.3 |   175.5% |   252.5% |
| argparse      |     1.28 |     130.3 |   136.2% |   118.2% |

使用编译参数 `-s -DNDEBUG -O2` 得到的结果如下

| Name          | Time (s) | Size (KB) | Time (%) | Size (%) |
| ------------- | -------: | --------: | -------: | -------: |
| ArgLite Mini  |     1.36 |      54.3 |   100.0% |   100.0% |
| ArgLite Full  |     1.56 |      62.3 |   114.7% |   114.7% |
| *ArgLite Sub* |     2.11 |      90.3 |   155.1% |   166.3% |
| CLI11         |     1.75 |     498.3 |   128.7% |   917.7% |
| cxxopts       |     2.67 |     118.3 |   196.3% |   217.9% |
| args          |     2.91 |     150.3 |   214.0% |   276.8% |
| argparse      |     1.77 |      74.3 |   130.1% |   136.8% |

可见 ArgLite 确实是最轻量的，精简版的优势特别明显。即便是使用了各种接口的完整示例, ArgLite 的成绩仍可以和其他库的最简程序比较，甚至表现更好。 argparse 的表现也很好，只是稍差于 ArgLite 完整版，属于同一个水平。 CLI11 得益于链接了现成的库文件，编译时间较短（还是比不上 ArgLite），但二进制体积最大。 cxxopts, args 的表现一般般，只是比全能库 CLI11 轻而已，而且因为不能链接现成的库文件，编译时间垫底。

## 运行时内存消耗

因为实际应用中命令行参数不会传非常多的选项参数，程序也不会定义非常多的选项，有几十个选项的已经是很复杂的应用了。但解析几十个选项的工作量还是非常低，哪怕是 Python 这种低性能语言，都能在极短的时间内完成，内存消耗也很低。位置参数可能会传很多，因为很多程序会用剩余位置参数获取文件。不过即便传入上千个文件，依然是很轻量的任务。所以测试这种库的运行时性能并没有多大实际意义。做这个测试只是为了更全面地对比这些库。在绝大多数情况下，你没必要根据这个测试结果判断哪个库更合适。哪怕是表现最差的，在实际应用中也无法感受到它的表现更差。

本测试使用 `valgrind` 的 `memcheck`, `massif` 工具测试，测试的二进制是上一个测试中用 `-s -DNDEBUG -O2` 编译得到的。因为 C++ 的运行时本身就要占用一定的内存，所以引入一个 Hello World 程序作为基准。各个库的成绩是减去这个基准后的结果。传入的基础参数是 `-v -c 123 outfile infile1 infile2`，并测试在此基础上传入更多的位置参数。测试结果包括: 堆内存的分配次数、总分配大小、峰值大小。

| Name         | Allocs | Allocated (KB) | Peak Heap (KB) |
| ------------ | -----: | -------------: | -------------: |
| Baseline     |      2 |          75.00 |          75.02 |
| ArgLite Mini |     20 |           1.37 |           0.16 |
| ArgLite Full |     22 |           1.45 |           0.16 |
| CLI11        |    133 |           8.14 |           5.96 |
| cxxopts      |    109 |           9.94 |           8.25 |
| args         |     41 |           2.19 |           0.96 |
| argparse     |     54 |           4.37 |           3.10 |

基础参数 + 1000 个位置参数:

| Name         | Allocs | Allocated (KB) | Peak Heap (KB) |
| ------------ | -----: | -------------: | -------------: |
| Baseline     |      2 |          75.00 |          75.02 |
| ArgLite Mini |     37 |          73.21 |          48.88 |
| ArgLite Full |     39 |          73.29 |          48.88 |
| CLI11        |    158 |         183.01 |         121.18 |
| cxxopts      |    125 |         232.06 |         134.98 |
| args         |     50 |          97.31 |          76.38 |
| argparse     |     76 |         254.48 |         160.98 |

基础参数 + 10000 个位置参数:

| Name         | Allocs | Allocated (KB) | Peak Heap (KB) |
| ------------ | -----: | -------------: | -------------: |
| Baseline     |      2 |          75.00 |          75.02 |
| ArgLite Mini |     45 |        1153.21 |         828.88 |
| ArgLite Full |     47 |        1153.29 |         828.88 |
| CLI11        |    170 |        2624.26 |        1722.10 |
| cxxopts      |    133 |        3393.31 |        1856.24 |
| args         |     54 |        1338.56 |        1078.00 |
| argparse     |     87 |        3251.03 |        2038.52 |

可见 ArgLite 的表现最好，总共申请的内存还没别的库的峰值内存多，申请次数也更少。传入少量参数时, ArgLite 的优势更明显。不过这时的优势也最没意义，因为消耗最多的也远比不上 C++ 的运行时消耗。因为两个版本获取位置参数的接口一样，所以内存使用情况几乎一样。两者的差别在于获取带值选项，而这类选项在实际使用中不会出现非常多，所以在极端情况下，完整版的运行时表现也和精简版几乎一样。主要是完整版的编译耗时更长，二进制体积更大。

args 的表现也很好，虽然它在上一个环节的表现一般般。 CLI11 作为重型库，虽然编译时可以通过链接现成的库文件缩短编译时间，但运行时的资源消耗很符合重型库的特点。在上个环节表现优异的 argparse 在这个环节的表现一般，并且随着传入的参数增加，它消耗内存的速度更快，导致在传入 1002 个位置参数时变成了消耗内存最多的。 cxxopts 号称轻量库，但表现比 CLI11 还差。并且正如之前说的，它默认会引入 `<regex>` 这种重量库，一点都不轻量。而且不但在轻量性上表现不佳，它还是参与测试的库中唯一不支持子命令的。

## 总结

因为运行时的表现对实际应用来说没多大意义，所以这里主要根据编译时的表现总结。

ArgLite 确实在轻量方面表现最好，无论是编译时还是运行时都是如此，而且提供了易用的接口和美观的输出。 argparse 在轻量和易用方面也做得好，只是它的输出不够友好。 CLI11 也有易用的接口，但它是个全能库，对于简单和中等程序来说太重了。其他两个库无论是轻量还是接口易用性都表现得不好，比不上其他库。特别是 cxxopts, 禁用 `<regex>` 后表现仍是一般，不禁用的话结果会翻倍。

当然，易用性和美观都是相对主观的指标，只有轻量性可以用客观指标衡量。我已经提供了测试文件，你可以很轻松地对比谁更易用。除了 CLI11，其他库也非常容易安装，你也可以很轻松地编译测试文件，然后对比它们的输出信息。如果你需要格式化的输出，那么确实只有 ArgLite 提供了良好的支持，argparse 有很简单的支持，其他库没有。

ArgLite 最大的缺点是出现的时间还很短，只是通过了项目自己的测试，没在实际使用中得到充分测试，不适合用于严肃的项目。所以完整版只是证明这个库有用于中等复杂度的项目的潜力，但目前还不合适。精简版本来就是用于简单程序，对于这类程序，目前的可靠性已经足够。
