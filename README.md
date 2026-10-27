# 实验一：带键盘事件的计算器（Qt）

东莞理工学院计算机科学与技术学院《Qt 应用程序开发》实验一。

使用 Qt Widgets + Qt Designer 实现的四则运算计算器，鼠标与键盘共用同一套输入/运算逻辑，
并针对连续小数点、连续运算符、除数为 0、计算完成后继续输入、连续运算、退格等边界情况做了处理。

## 功能

- 加、减、乘、除四则运算，支持小数输入
- 退格（⌫）、清除（C）、等号（=）
- 键盘输入：数字、`.`、`+ - * /`、`Enter`/`=`、`Backspace`、`Esc`
- 表达式行显示当前算式，结果行显示输入与结果
- 除数为 0 时显示“错误 / 除数不能为 0”，输入数字或清除后自动复位
- 深色样式表，运算符橙色分区，按钮带悬停/按下反馈

## 目录结构

```
Calculator-Exp1/
├── Calculator.pro          # qmake 工程文件
├── main.cpp                # 程序入口
├── calculator.h            # Calculator 类声明与状态变量
├── calculator.cpp          # 信号槽、输入处理、运算与键盘事件
├── calculator.ui           # Qt Designer 界面（布局、控件属性、样式表）
├── tests/
│   ├── tests.pro           # 测试工程
│   └── test_calculator.cpp # QtTest 自动化测试
├── screenshots/            # 运行截图（测试自动生成）
└── README.md
```

## 构建与运行

本机实测环境：Windows 11 + Qt 5.12.11 (MinGW 7.3 64-bit)。代码使用 Qt 5/Qt 6 兼容写法。

### Qt Creator

1. 打开 `Calculator.pro`，选择 MinGW 64-bit 套件；
2. 构建并运行。

### 命令行（Qt 5.12.11 示例）

```bat
set PATH=D:\Qt\5.12.11\mingw73_64\bin;D:\Qt\Tools\mingw730_64\bin;%PATH%
mkdir build && cd build
qmake ..\Calculator.pro -spec win32-g++
mingw32-make -j8
windeployqt release\Calculator.exe
release\Calculator.exe
```

## 自动化测试

测试会创建真实窗口，通过 `QTest::mouseClick` / `QTest::keyClick` 驱动界面，
断言显示结果，并用 `QWidget::grab()` 将关键状态保存到 `screenshots/`。

```bat
set PATH=D:\Qt\5.12.11\mingw73_64\bin;D:\Qt\Tools\mingw730_64\bin;%PATH%
mkdir build-tests && cd build-tests
qmake ..\tests\tests.pro -spec win32-g++
mingw32-make -j8
cd ..
build-tests\release\CalculatorTests.exe -o test-report.txt,txt
```

实测结果：14 项测试全部通过（正常运算、重复小数点、连续运算符、连续运算、结果后输入、
除零、退格、清除、鼠标键盘混合输入、焦点配置）。

## 按键映射

| 键盘 | 功能 |
| --- | --- |
| `0`–`9`（含小键盘） | 输入数字 |
| `.` / `,` | 小数点 |
| `+` `-` `*` `/` | 加、减、乘、除 |
| `Enter` / `=` | 计算结果 |
| `Backspace` | 退格 |
| `Esc` / `Delete` | 清除 |

## 设计要点

- **状态管理**：`m_storedValue`（第一操作数/中间结果）、`m_pendingOp`（待执行运算符）、
  `m_waitingForOperand`（等待新操作数）、`m_justEvaluated`（刚按过等号）、`m_error`（错误状态）。
- **键盘复用**：`keyPressEvent()` 只做按键到方法的映射，实际逻辑与按钮槽函数共用
  `appendDigit()`、`appendDot()`、`applyOperator()`、`backspace()`、`clearAll()`。
- **焦点**：显示框与所有按钮 `focusPolicy = NoFocus`，主窗口 `StrongFocus`，
  避免子控件吞掉键盘事件。
- **重复小数点**：`appendDot()` 中通过 `text.contains('.')` 拦截；等待新操作数时自动补 `0.`。
- **连续运算符**：`applyOperator()` 在 `m_pendingOp` 非空且 `m_waitingForOperand` 为真时只替换运算符。
- **连续运算**：第二次按运算符时先 `calculate()` 得到中间结果，再记录新运算符（从左到右计算）。

## 截图

| 文件 | 内容 |
| --- | --- |
| `01-main-ui.png` | 主界面 |
| `02-mouse-decimal-add.png` | 鼠标：12.5 + 3.75 = 16.25 |
| `03-keyboard-decimal-add.png` | 键盘：12.5 + 3.75 = 16.25 |
| `04-continuous-operation.png` | 连续运算：2 + 3 × 4 = 20 |
| `05-divide-by-zero.png` | 除数为 0 |
| `06-backspace.png` | 输入过程退格 |
| `07-clear-and-reuse.png` | 清除后重新计算 |
