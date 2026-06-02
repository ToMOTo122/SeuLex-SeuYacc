# SeuLex & SeuYacc — 编译器前端工具链

> 东南大学编译原理课程设计：从零实现的词法分析器生成器 (SeuLex) 与语法分析器生成器 (SeuYacc)

## 项目简介

本项目实现了两个编译器前端生成工具：

- **SeuLex**：词法分析器生成器。读入 `.l` 格式的词法规则文件，自动生成完整的词法分析器 C 源码 (`yylex.c`)。核心算法包括 Thompson NFA 构造、子集构造法 (NFA→DFA)、Hopcroft DFA 最小化、最长匹配词法扫描驱动。
- **SeuYacc**：语法分析器生成器。读入 `.y` 格式的语法规则文件，自动生成完整的 LALR(1) 语法分析器 C 源码 (`yyparse.c`) 及 token 定义头文件 (`minic.tab.h`)。核心算法包括 FIRST/FOLLOW 集计算、LR(1) DFA 构造、同核合并 LALR(1)、分析表填充（含移进-归约冲突的优先级/结合性解决）、语义动作伪变量翻译。

两个工具协同工作，可为目标语言 MiniC 生成完整的编译器前端（词法分析 → 语法分析 → 语义检查）。

## 目录结构

```
.
├── seulex/                    # SeuLex 源码（词法分析器生成器）
│   ├── SeuLex.cpp             #   主程序入口（6 阶段流水线）
│   ├── LexParser.cpp / .h     #   .l 文件解析器
│   ├── REProcessor.cpp / .h   #   正则表达式处理（规范化 / 加点 / 中缀转后缀）
│   ├── NFABuilder.cpp / .h    #   Thompson NFA 构造（含字符类紧凑标记）
│   ├── DFABuilder.cpp / .h    #   子集构造法 NFA→DFA
│   ├── DFAMinimizer.cpp / .h  #   Hopcroft DFA 最小化
│   └── LexCodeGen.cpp / .h    #   词法分析器代码生成 → yylex.c
│
├── seuyacc/                   # SeuYacc 源码（语法分析器生成器）
│   ├── SeuYacc.cpp            #   主程序入口（4 阶段流水线）
│   ├── GrammarParser.cpp / .h #   .y 文件解析器
│   ├── DFAGenerator.cpp / .h  #   FIRST/FOLLOW / LR(1) / LALR(1) DFA
│   └── Emitter.cpp / .h       #   语法分析器代码生成 → yyparse.c
│
├── 课程资料/                   # 课程提供的输入文件
│   ├── minic.l                #   MiniC 词法规则（SeuLex 输入）
│   └── minic.y                #   MiniC 语法规则（SeuYacc 输入）
│
├── 运行时 C 库（被生成的代码依赖）
│   ├── names.c / .h           #   字符串池（标识符复用）
│   ├── symtab.c / .h          #   作用域符号表
│   ├── types.c / .h           #   类型系统（int / float / array / record / func）
│   ├── check.c / .h           #   类型检查（赋值 / 算术 / 关系 / 函数调用）
│   └── minic.tab.h            #   Token 定义（SeuYacc 自动生成）
│
├── 驱动程序
│   ├── demo_lexer.c           #   Token 流打印驱动
│   ├── demo_parser.c          #   Parser 归约跟踪驱动（YY_TRACE 模式）
│   ├── minic_driver.c         #   MiniC 编译器驱动
│
├── 测试文件
│   ├── test.minic             #   全局变量 + 函数 + if-else
│   ├── test2.minic            #   嵌套 if-else（悬挂 else 问题）
│   ├── demo_test.minic        #   递归阶乘
│
├── 构建脚本
│   ├── build.py               #   编译 SeuLex.exe / SeuYacc.exe（MSVC）
│   ├── build_demo.py          #   一键构建演示程序（工具→生成→编译→运行）
│   ├── build_minic.py         #   编译 minic.exe
│   ├── build_msvc.bat         #   MSVC 批处理编译（备选）
│   └── Makefile               #   g++ Makefile（Linux / MinGW 备选）
│
├── 生成的产物（可随时重新生成）
│   ├── yylex.c                #   ← SeuLex 生成
│   ├── yyparse.c              #   ← SeuYacc 生成
│   ├── minic.tab.h            #   ← SeuYacc 生成
│   ├── SeuLex.exe             #   ← build.py 编译
│   ├── SeuYacc.exe            #   ← build.py 编译
│   ├── demo_lexer.exe         #   ← build_demo.py 编译
│   ├── demo_parser.exe        #   ← build_demo.py 编译
│   └── minic.exe              #   ← build_minic.py 编译
│
└── shared/                    # [未使用] 早期 C++ 版设计残留
    ├── tokens.h               #   C++ Token 枚举（运行时用 minic.tab.h）
    ├── AST.h                  #   AST 节点（项目未实现 AST 构建）
    └── SymbolTable.h          #   C++ 符号表（运行时用 symtab.h）
```

## 核心算法

### SeuLex 处理流程

```
.l 文件 → LexParser → LexSpec
    ↓
expandDefs → normalizeRE → addDots → infixToPostfix  (RE 处理链)
    ↓
Thompson NFA 构造 (每条规则独立构造，合并到统一 NFA)
    ↓
子集构造法 NFA→DFA (ε-closure + move)
    ↓
Hopcroft DFA 最小化 (划分细化)
    ↓
LexCodeGen → yylex.c (转移表 + 最长匹配驱动循环 + 动作分发)
```

关键算法细节：
- **字符类紧凑标记**：`[A-Za-z]` 等大字符集用 0x80-0xFE 范围标记替代展开，避免 DFA 状态爆炸
- **通配符规则**：`.` 模式单独处理，不进入 DFA，在无匹配时作为兜底
- **最长匹配**：DFA 驱动循环记录最近接受状态，回溯到最长匹配位置

### SeuYacc 处理流程

```
.y 文件 → GrammarParser → GrammarSpec
    ↓
FIRST / FOLLOW 集计算 (不动点迭代)
    ↓
LR(1) DFA 构造 (closure + goto)
    ↓
同核合并 → LALR(1) DFA
    ↓
分析表填充 (ACTION + GOTO, 移进-归约冲突按优先级/结合性解决)
    ↓
Emitter → yyparse.c + minic.tab.h
    ($$ → yy_result, $N → yy_v[N-1], $<tag>N → yy_v[N-1].tag)
```

关键算法细节：
- **悬挂 else 解决**：`%nonassoc LOW` / `%nonassoc ELSE` 使 ELSE 优先级高于 LOW，优先移进
- **LALR(1) 合并**：相同核心 (prodId, dotPos) 的 LR(1) 状态合并，lookahead 取并集
- **语义动作翻译**：自动将 Yacc 风格的 `$$`、`$1`、`$<tag>N` 转换为 C 代码

## 编译与运行

### 前置条件

- **Visual Studio 2022** (Community 或以上)，含"使用 C++ 的桌面开发"工作负载
- **Python 3**（用于构建脚本）

### 快速开始（一键构建演示）

```powershell
# 打开 "x64 Native Tools Command Prompt for VS 2022"，进入项目目录

# 一键构建所有演示程序
python build_demo.py

# SeuLex 演示：打印 token 流
demo_lexer.exe test.minic

# SeuYacc 演示：打印归约序列
demo_parser.exe test.minic
```

`build_demo.py` 自动完成：编译 SeuLex/SeuYacc → 生成 yylex.c/yyparse.c → 编译 demo_lexer.exe/demo_parser.exe

### 分步构建

```powershell
# 1. 编译生成工具
python build.py seulex
python build.py seuyacc

# 2. 生成分析器代码
SeuLex.exe 课程资料/minic.l -o yylex.c
SeuYacc.exe 课程资料/minic.y -o yyparse.c

# 3. 编译 MiniC 编译器
python build_minic.py

# 4. 运行
minic.exe test.minic
```

### Linux / MinGW 环境

```bash
make clean && make        # 编译 SeuLex.exe 和 SeuYacc.exe
# 然后手动编译生成的 C 代码
```

## 使用说明

### SeuLex

```
SeuLex.exe <input.l> [-o output.c]
```

- `input.l`：词法规则文件（Lex/flex 格式）
- `-o output.c`：输出文件名（默认 `yylex.c`）

输入格式：
```
%{  用户代码（直接复制到输出开头）  %}
定义名  正则表达式
%%
规则正则  { 动作代码 }
规则正则  动作代码;
%%
用户函数（复制到输出末尾）
```

### SeuYacc

```
SeuYacc.exe <input.y> [-o output.c]
```

- `input.y`：语法规则文件（Yacc/Bison 格式）
- `-o output.c`：输出文件名（默认 `yyparse.c`）

支持的声明：`%token`、`%left`、`%right`、`%nonassoc`、`%union`、`%type`、`%start`

## MiniC 语言

MiniC 是一个精简的 C 子集语言，支持：

- **类型**：`int`、`float`、数组 (`type*`)、结构体 (`struct { ... }`)
- **声明**：全局变量、函数声明（含参数）、局部变量
- **表达式**：算术 (`+ - * /`)、比较 (`==`)、函数调用、数组/记录访问、一元负号
- **语句**：`if`/`else`、`return`、赋值、代码块 `{ ... }`
- **语义检查**：类型匹配验证、未定义符号检测、函数参数数量/类型校验

## 测试结果

| 测试文件 | 词法分析 | 语法分析 | 说明 |
|----------|----------|----------|------|
| test.minic | ✅ 正确 | ✅ Parse Successful | 全局变量 + 函数 + if-else |
| test2.minic | ✅ 正确 | ✅ Parse Successful | 嵌套 if-else（悬挂 else 正确解决） |
| demo_test.minic | ✅ 正确 | ✅ 归约序列完整 | 递归阶乘（运行时语义检查报错：函数声明作用域问题） |

## 技术参数

| 项目 | 数值 |
|------|------|
| SeuLex 输入规则数 | 26 条 + 3 个定义 |
| 最小化 DFA 状态数 | 45 |
| SeuYacc 输入产生式数 | 50 条 |
| LR(1) 状态数 | 200 |
| LALR(1) 状态数 | 95 |
| FIRST 集数量 | 19 个非终结符 |

## 清理编译产物

```powershell
# 删除所有中间产物和可执行文件
del /s *.obj
del SeuLex.exe SeuYacc.exe demo_lexer.exe demo_parser.exe minic.exe
del yylex.c yyparse.c minic.tab.h
```

重新构建只需再次运行 `python build_demo.py`。

## 许可证

本项目为东南大学编译原理课程设计作品，仅供学习和教学使用。