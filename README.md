# SeuLex + SeuYacc 编译器生成工具

基于中期详细设计方案实现的编译器前端工具链：词法分析器生成器 (SeuLex) 和语法分析器生成器 (SeuYacc)。目标语言为 MiniC。

## 目录结构

```
.
├── seulex/                  SeuLex 源码 (词法分析器生成器)
│   ├── SeuLex.cpp               主程序 (6阶段流水线)
│   ├── LexParser.cpp/h          .l 文件解析 → LexSpec
│   ├── REProcessor.cpp/h        正规表达式处理 (规范化/加点/中缀转后缀)
│   ├── NFABuilder.cpp/h         Thompson NFA 构造 (支持字符类紧凑表示)
│   ├── DFABuilder.cpp/h         子集构造法 NFA→DFA
│   ├── DFAMinimizer.cpp/h       DFA 最小化 (划分细化)
│   └── LexCodeGen.cpp/h         代码生成 → yylex.c
│
├── seuyacc/                 SeuYacc 源码 (语法分析器生成器)
│   ├── SeuYacc.cpp               主程序 (4阶段流水线)
│   ├── GrammarParser.cpp/h       .y 文件解析 → GrammarSpec
│   ├── DFAGenerator.cpp/h        FIRST/FOLLOW / LR(1) / LALR(1)
│   └── Emitter.cpp/h             代码生成 → yyparse.c (含 $N/$$ 翻译)
│
├── shared/                  共享头文件
│   ├── tokens.h                  Token 类型枚举
│   ├── AST.h                     AST 节点定义与打印
│   └── SymbolTable.h             作用域栈式符号表
│
├── 课程资料/                课程提供的参考文件
│   ├── minic.l                   MiniC 词法规则 (SeuLex 输入)
│   ├── minic.y                   MiniC 语法规则 (SeuYacc 输入)
│   ├── c99.l / c99.y             C99 参考规则
│   └── *.pdf / *.pptx / *.docx   课程文档
│
├── SeuLex.exe               SeuLex 可执行文件
├── SeuYacc.exe              SeuYacc 可执行文件
├── yylex.c                  生成的 MiniC 词法分析器
├── yyparse.c                生成的 MiniC 语法分析器
│
├── names.c/h                运行时：字符串池
├── symtab.c/h               运行时：作用域符号表
├── types.c/h                运行时：类型系统
├── check.c/h                运行时：类型检查
├── minic.tab.h              运行时：Token 定义
│
├── minic_driver.c           编译器驱动程序
├── minic.exe                完整的 MiniC 编译器
├── test.minic               MiniC 测试程序
│
├── build.py                 编译脚本 (MSVC)
├── build_minic.py           编译 minic.exe 脚本
├── build_msvc.bat           备选：MSVC 批处理编译
├── Makefile                 备选：MinGW Make 编译
└── README.md                本文件
```

## 编译方法

**前置条件**：安装 Visual Studio 2022 (Community 或以上)，包含"使用 C++ 的桌面开发"工作负载。

### 第一步：打开 VS 开发者命令行

从开始菜单打开 **"x64 Native Tools Command Prompt for VS 2022"**，进入项目目录。

### 第二步：编译工具

```cmd
python build.py seulex    REM 编译 SeuLex.exe
python build.py seuyacc    REM 编译 SeuYacc.exe
```

### 第三步：生成分析器

```cmd
SeuLex.exe 课程资料/minic.l -o yylex.c
SeuYacc.exe 课程资料/minic.y -o yyparse.c
```

### 第四步：编译 MiniC 编译器

```cmd
python build_minic.py
```

### 第五步：测试

```cmd
minic.exe test.minic
```

输出 `=== Compilation Successful ===` 表示词法分析、语法分析、语义检查全部通过。

## 使用方法（详细）

### SeuLex 命令行

```
SeuLex.exe <input.l> [-o output.c]
```

处理流程：
1. 解析 .l 文件 → LexSpec（定义表、规则列表、用户代码）
2. RE 规范化（展开 `?` `+` `[...]` 为 `|` `·` `*`，字符类使用紧凑标记避免状态爆炸）
3. 中缀→后缀转換（算符优先法）
4. Thompson 算法构造 NFA（每个规则独立构造，再合并）
5. 子集构造法 NFA→DFA（ε-closure + move）
6. Hopcroft 算法最小化 DFA
7. 生成 yylex.c（转移表 + 最长匹配驱动循环）

### SeuYacc 命令行

```
SeuYacc.exe <input.y> [-o output.c]
```

处理流程：
1. 解析 .y 文件 → GrammarSpec（符号表、产生式、语义动作）
2. 计算 FIRST / FOLLOW 集（不动点迭代）
3. 构造 LR(1) DFA（闭包 + Goto）
4. 合并同核 → LALR(1) DFA
5. 填充 ACTION / GOTO 表（冲突按优先级解决）
6. 翻译 `$$` `$1` `$<tag>N` 等伪变量为 C 代码
7. 生成 yyparse.c（分析表 + LALR 驱动循环 + 语义动作）

### minic.exe 命令行

```
minic.exe <source.minic>
```

对 MiniC 源程序执行完整的词法分析→语法分析→语义检查（类型检查、符号表管理）。

## 核心算法

| 阶段 | 算法 |
|------|------|
| RE 处理 | 展开 `?` `+` `[...]` → `\|` `·` `*`；中缀转后缀（算符优先） |
| NFA 构造 | Thompson 算法；字符类使用紧凑标记（避免状态爆炸） |
| NFA→DFA | 子集构造法（ε-closure + move）；通配符规则特殊处理 |
| DFA 最小化 | 划分细化算法（按终态动作 + 转移行为分离） |
| 词法扫描 | 最长匹配 + 回溯 + 通配符回退机制 |
| FIRST/FOLLOW | 不动点迭代 |
| 语法 DFA | LR(1) 闭包/Goto → 合并同核 = LALR(1) |
| 分析表 | ACTION/GOTO 表填充；移进-归约冲突按优先级/结合性解决 |
| 语义动作 | `$$`→yy_result, `$N`→yy_v[N-1], `$<tag>N`→yy_v[N-1].tag |

## MiniC 语言

支持的语言特性：
- **类型**：`int`, `float`, 数组 (`type*`), 结构体 (`struct`)
- **表达式**：算术 (`+` `-` `*` `/`)、比较 (`==`)、函数调用、数组/记录访问
- **语句**：`if` / `else`、`while`、`return`、赋值、代码块 `{}`
- **函数**：声明、调用、参数传递

测试程序 `test.minic` 覆盖了变量声明、函数定义、赋值、函数调用、if-else、return 等特性。

## 清理

```cmd
del /s *.obj
del minic.exe
```
