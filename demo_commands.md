# SeuLex + SeuYacc 验收演示步骤

在项目根目录下执行所有命令。

## 第一步：编译工具

```powershell
python build.py seulex
python build.py seuyacc
```

## 第二步：构建演示程序

```powershell
python build_demo.py
```

这一步自动完成：生成 yylex.c / yyparse.c → 编译 demo_lexer.exe → 编译 demo_parser.exe

## 第三步：SeuLex 演示 —— Token 流

```powershell
./demo_lexer.exe demo_test.minic
```

输出每个 token 的行号、类型、词素，例如 `INT 'int'`, `NAME 'fact'`, `NUMBER '5'`。

## 第四步：SeuYacc 演示 —— 归约序列

```powershell
./demo_parser.exe demo_test.minic
```

每次归约打印 `[reduce N] LHS -> RHS`，展示自底向上的移进-归约过程。

## 演示数据

| 阶段 | 输入 | 关键数字 |
|------|------|----------|
| SeuLex | minic.l (26条规则) | 34 状态最小化 DFA |
| SeuYacc | minic.y (47条产生式) | 21 LALR(1) 状态, 19个非终结符 FIRST 集 |

## 附：完整编译器

```powershell
python build_minic.py
./minic.exe demo_test.minic
```
