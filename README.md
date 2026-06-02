# SeuLex & SeuYacc — 编译器前端工具链

> 东南大学编译原理课程设计：从零实现的词法分析器生成器 (SeuLex) 与语法分析器生成器 (SeuYacc)
> 
> 项目成员：王辰文，李雅蓉，杨皓如

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

## 编译过程可视化 (minic-visualizer)

### 简介

`minic-visualizer` 是一个基于 React + Vite + Tailwind CSS 的前端可视化项目，将 SeuLex 词法分析和 SeuYacc 语法分析的过程以动画形式呈现，帮助理解编译器前端的工作原理。

### 项目结构

```
minic-visualizer/
├── index.html                     # 入口 HTML
├── vite.config.js                 # Vite 配置 (React + Tailwind)
├── package.json                   # 依赖: react, react-d3-tree, react-icons, react-router, tailwindcss
├── src/
│   ├── main.jsx                   # React 入口
│   ├── index.css                  # Tailwind 导入 + 自定义动画
│   ├── App.jsx                    # 主布局 + 路由 (主页 / 原理页)
│   ├── hooks/
│   │   └── useMiniCCompiler.js    # 编译核心 hook (分步动画控制)
│   ├── services/
│   │   ├── miniCLexer.js          # MiniC 词法分析器 (JS 实现, 基于 minic.l)
│   │   └── miniCParser.js         # MiniC 递归下降语法分析器 (JS 实现, 基于 minic.y)
│   └── components/
│       ├── LexicalTrace.jsx        # 词法分析动画 (源码扫描 + DFA 状态转移 + Token 流)
│       ├── ParseTrace.jsx          # 语法分析动画 (移进-归约栈可视化)
│       ├── ASTVisualization.jsx    # 语法树可视化 (文本/图形双模式, react-d3-tree)
│       ├── SymbolTable.jsx         # 符号表展示
│       ├── TokenTable.jsx          # Token 详细表格 (筛选/排序)
│       ├── PhaseVisualization.jsx  # 编译阶段卡片容器
│       ├── CodeInput.jsx           # 代码输入 + 示例选择
│       ├── HowItWorks.jsx          # 工作原理说明页
│       └── Footer.jsx              # 页脚
```

### 运行方式

```bash
cd minic-visualizer

# 安装依赖
npm install

# 开发模式
npm run dev
# 打开 http://localhost:5173

# 生产构建
npm run build
```

### 核心实现：分步动画 Hook

`useMiniCCompiler` 是整个动画系统的核心，管理编译过程的分步执行状态：

**状态模型：**
```
idle → lexing → parsing → done
```

**核心状态：**
- `phase` — 当前阶段 (`idle` | `lexing` | `parsing` | `done`)
- `charIndex` — 词法扫描到的字符位置（用于源码高亮）
- `tokenIndex` — 已识别的 Token 数量（用于 Token 流动画）
- `parseStepIndex` — 语法分析步骤编号（用于移进-归约动画）
- `isPlaying` — 自动播放开关
- `speed` — 播放速度 (1-10)

**核心方法：**
- `startCompilation(code)` — 初始化编译，进入词法阶段
- `stepForward()` — 手动单步前进
- `togglePlay()` — 播放/暂停切换
- `setSpeed(n)` — 设置播放速度
- `reset()` — 重置所有状态
- `quickCompile(code)` — 跳过动画，直接完成编译

**使用方式：**
```jsx
const {
  tokens, parseEvents, astTree, symbolTable,  // 编译结果
  phase, charIndex, tokenIndex, parseStepIndex, // 动画位置
  isPlaying, speed,                            // 播放控制
  startCompilation, stepForward, togglePlay,    // 控制方法
  setSpeed, reset,
} = useMiniCCompiler();

// 开始编译
startCompilation(code);

// 动画控制
togglePlay();    // 播放/暂停
stepForward();   // 单步前进
```

### 词法分析动画 (LexicalTrace)

参考 `visualization.html` 的词法分析板块实现，包含三个可视化区域：

1. **源代码扫描区**：原始代码展示，已扫描部分高亮（深绿色背景），当前位置用黄色标记，未扫描部分灰色
2. **DFA 状态转移**：显示 `S0 ─'token'→ ✓ TYPE`，模拟 DFA 接受过程
3. **Token 流出区**：已识别的 Token 逐个动画显示，每个 Token 带颜色分类标签

词法完成后，可展开查看 Token 详细表格（筛选、排序）。

### 语法分析动画 (ParseTrace)

基于 LALR(1) 移进-归约分析的可视化：

1. **栈与剩余输入**：左右并排显示，栈顶高亮，当前读入 Token 黄色标记
2. **当前动作**：移进（蓝色）、归约（绿色）、接受（深绿色）用不同颜色标识
3. **完整轨迹**：可折叠的完整解析步骤日志

### 语法树可视化 (ASTVisualization)

提供两种视图模式：
- **文本视图**：树状文本打印，缩进表示层级，默认模式
- **图形视图**：react-d3-tree 渲染的交互式树图，可拖拽/缩放

AST 由归约事件序列自动构建——每次归约产生一个子树节点。

### 语义分析 (SymbolTable)

从 Token 序列中自动提取符号信息：
- 变量名、类型（根据前置类型关键字推断）
- 作用域（根据花括号嵌套层级判断全局/局部）
- 函数声明（检测 `NAME(...)` 模式）

### 与 SeuLex/SeuYacc 的对应关系

| 可视化模块 | 对应的 C 生成代码 | JS 实现方式 |
|-----------|------------------|------------|
| miniCLexer.js | yylex.c (SeuLex 生成) | JS 手写词法扫描器，规则来自 minic.l |
| miniCParser.js | yyparse.c (SeuYacc 生成) | JS 递归下降解析器，文法来自 minic.y |
| LexicalTrace | DFA 扫描驱动 | 逐 Token 动画 + DFA 状态转移文字 |
| ParseTrace | yyparse 分析表驱动 | 逐事件动画 + 栈重建 |
| ASTVisualization | — (本项目未实现 AST C 代码) | 从归约事件序列自动构建 |
| SymbolTable | symtab.c | 从 Token 序列提取 |

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
