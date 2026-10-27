# 通用 CLI 框架（模板）

一个面向嵌入式调试/测试的表驱动命令行框架。框架层（`cli_io` / `cli_parser` / `cli_command`）不依赖任何业务代码，应用层只需要实现命令处理函数并把它们登记到命令表里。

当前目录是从既有工程中剥离出来的**原型模板**，用于新项目起步；配套的旧框架代码计划报废。

---

## 1. 特性

- 静态命令表，无动态注册、无堆分配
- `!`（DO）、`?`（READ）、`=`（WRITE）三种命令类型
- 参数支持十进制、负数、十六进制（`0x...`）
- 框架统一校验参数**个数**，命令自行校验参数**取值**
- `printf` 后端可在编译期切换：内建格式化 / 标准库 `vprintf`
- 端口层只有 4 个函数，便于移植
- 固定大小缓冲区，无动态内存，超长输入按规则截断

---

## 2. 文件结构

| 文件 | 层 | 职责 |
|---|---|---|
| `cli_config.h` | 配置 | 缓冲区大小、参数上限、波特率、printf 后端选择 |
| `cli_io.h` / `cli_io.c` | 端口 + 输出 | `cli_io_*`、`cli_printf`、`cli_itoa`、`cli_init` |
| `cli_parser.h` / `cli_parser.c` | 解析 | `cli_read_line`、`cli_parse_line` |
| `cli_command.h` / `cli_command.c` | 分发 | 命令表类型、`cli_execute_line`、`cli_run` |
| `cli_app_cmds.h` / `cli_app_cmds.c` | 应用 | **使用者主要修改的文件** |

框架层不包含任何业务头文件。`cli_app_cmds.c` 是唯一需要按项目改写的文件。

---

## 3. 快速开始

### 3.1 加入工程

1. 把 `cli_io.c`、`cli_parser.c`、`cli_command.c`、`cli_app_cmds.c` 加入编译；
2. 把本目录加入 include 路径；
3. 确保 `cli_app_cmds.c` 中 `g_cli_command_table[]` 有且仅有一处定义。

### 3.2 提供入口

```c
#include "cli_config.h"
#include "cli_io.h"

int main(void)
{
    if (cli_init(CLI_BAUD_RATE) != CLI_OK)
    {
        return 1;
    }

    cli_run();

    return 0;
}
```

- 在真实工程里，把 `cli_init()` / `cli_run()` 放在板级、外设初始化之后。
- `cli_init()` 只初始化端口层，不做业务初始化。
- `cli_run()` 会阻塞，直到收到 `exit!` 或输入结束（EOF）。

### 3.3 运行

编译运行后输入：

```text
help?
```

应看到当前命令表。随后可以试：

```text
example_get?
example_set=123
example_get?
exit!
```

---

## 4. 命令语法

命令模板（命令表中的 `name`）以三种终止符之一结尾：

| 终止符 | 类型 | 示例 | 参数处理 |
|---|---|---|---|
| `!` | DO | `exit!` | 不解析，`param_count` 恒为 0 |
| `?` | READ | `example_get?` | 解析，按表项 `min/max` 校验 |
| `=` | WRITE | `example_set=123` | 解析，按表项 `min/max` 校验 |

规则：

- 输入必须逐字符匹配模板，直到终止符；命中表中第一条命令后立即执行并返回。
- 参数以空格或 Tab 分隔，形如 `-5`、`0x1F`、`1.5`。
- 终止符之后的内容全部按参数处理。例如 `help?x` 会匹配 `help?`，然后把 `x` 当作参数，最终因 `help?` 的 `max_param_count = 0` 报参数错误。
- 空行被忽略；未知命令打印 `Invalid command!`。

---

## 5. 添加一条命令

以增加 `demo_set=` 为例，共三步。

### 5.1 写处理函数

在 `cli_app_cmds.c` 的 `// User commands` 区域添加：

```c
int cmd_demo_set(double *params, uint8_t param_count)
{
    uint32_t value;

    /* 参数个数由命令表的 min/max 保证，这里不再判断。 */
    (void)param_count;

    /* 取值合法性由命令自己负责：范围检查同时拦住 NaN。 */
    if (!((params[0] >= 0.0) && (params[0] <= (double)UINT32_MAX)))
    {
        cli_print_invalid_param();
        return CLI_EXEC_CONTINUE;
    }

    value = (uint32_t)params[0];
    cli_printf("demo=%u\n", value);

    return CLI_EXEC_CONTINUE;
}
```

### 5.2 登记到命令表

```c
{"demo_set=", "Sets the demo value.", "demo_set=123", 1, 1, cmd_demo_set},
```

字段顺序：`name`、`help`、`usage`、`min_param_count`、`max_param_count`、`handler`。

### 5.3 在头文件中声明

```c
int cmd_demo_set(double *params, uint8_t param_count);
```

命名约定：

- 框架符号用 `cli_` 前缀；
- 命令处理函数用 `cmd_<名字>_get/set`；
- 全局变量用 `g_` 前缀，文件内静态变量可用 `s_` 或直接 `static`。

---

## 6. 命令表字段

| 字段 | 类型 | 说明 |
|---|---|---|
| `name` | `const char *` | 命令模板，必须以 `!` / `?` / `=` 结尾 |
| `help` | `const char *` | `help?` 列表和参数错误提示中显示 |
| `usage` | `const char *` | 非空时，参数错误提示会额外打印 `Example: ...` |
| `min_param_count` | `uint8_t` | READ/WRITE 的参数个数下限 |
| `max_param_count` | `uint8_t` | READ/WRITE 的参数个数上限 |
| `handler` | `cli_cmd_handler_t` | 处理函数，签名见下 |

处理函数签名固定为：

```c
int handler(double *params, uint8_t param_count);
```

DO 命令不使用 `min/max`，请填 `0, 0`。

---

## 7. 核心约定

### 7.1 `params` 的生命周期

- `params` 是 `cli_execute_line()` 内部的临时数组，每次读入一行都会重新初始化。
- 处理函数只应**读取** `params`，不要保存指针供以后使用（函数返回后即失效）。
- `param_count` 是唯一可信的参数个数；不要用 `params[i]`（`i >= param_count`）去猜测缺省值。
- 当 `min_param_count = 0` 时，处理函数必须先判断 `param_count >= 1` 再访问 `params[0]`。

### 7.2 个数校验与取值校验

职责划分：

| 校验内容 | 负责方 |
|---|---|
| 参数个数是否在 `[min, max]` 内 | 分发器（框架） |
| 数值范围、NaN/Inf、业务合法性 | 命令处理函数 |
| 参数能否转成数值 | 解析器不判断合法性；无法解析时按 `atof` 规则取值 |

个数不合法时，分发器打印 `Invalid parameter!` 和该命令的 `name` / `help` / `usage`，并返回 `CLI_EXEC_PARSEFAIL`，不会调用处理函数。

### 7.3 DO 命令

- 解析器遇到 `!` 会立即返回，不再解析后面的内容。
- 分发器不校验 DO 命令的参数个数。
- 因此 `example_do! 1 2` 会正常执行，尾部内容被忽略。这是设计行为。

### 7.4 返回值与 `CLI_EXEC_PARSEFAIL`

处理函数只应返回：

| 返回值 | 含义 |
|---|---|
| `CLI_EXEC_CONTINUE` | 命令执行完毕，继续运行 |
| `CLI_EXEC_EXIT` | 请求 `cli_run()` 退出循环（`exit!` 使用） |

`cli_command.h` 中还定义了 `CLI_EXEC_PARSEFAIL`，当前情况如下：

```c
#define CLI_EXEC_CONTINUE   (0)
#define CLI_EXEC_EXIT       (1)
#define CLI_EXEC_PARSEFAIL  (0)   /* parse failed, currently does nothing */
```

- 它是**分发器**的返回值，不是处理函数的返回值，处理函数不要返回它。
- 分发器在两处返回它：参数个数不合法、命令未找到。
- 它的值当前与 `CLI_EXEC_CONTINUE` 相同，所以行为上也等价于“继续运行”。`cli_run()` 只判断 `!= CLI_EXEC_EXIT`，因此不会因为解析失败而退出。
- 这样定义是为了保留一个语义标记：将来如果需要让调用方区分“执行成功”和“解析失败”，把它的值改成例如 `2` 即可，`cli_run()` 不需要修改。
- 在它保持为 `0` 期间，调用方无法通过返回值区分这两种情况；需要区分时请先修改宏的值。

### 7.5 关键接口

| 接口 | 说明 |
|---|---|
| `int cli_init(unsigned long baud_rate)` | 初始化端口层，返回 `CLI_OK` / `CLI_ERR` |
| `void cli_run(void)` | 读取并分发命令，直到 `exit!` 或 EOF |
| `int cli_execute_line(const char *line)` | 分发单行，返回处理函数的值或 `CLI_EXEC_PARSEFAIL` |
| `void cli_print_invalid_param(void)` | 打印当前命令的参数错误信息 |
| `size_t cli_read_line(char *line, size_t size)` | 读取一行，返回长度或 `CLI_READ_EOF` |
| `int cli_parse_line(const char *input_line, const char *command_pattern, double *params, uint8_t *param_count)` | 用命令模板匹配一行并解析参数 |
| `void cli_printf(const char *fmt, ...)` | 格式化输出，格式支持见第 9 节 |

---

## 8. 参数解析

支持的形式：

```text
example_set=123         十进制
example_set=-5          负数
example_set=1.5         浮点
example_get_addr? 0x1F  十六进制
```

限制与行为：

- 单行最多 `CLI_LINE_MAX - 1`（默认 63）个字符，超出部分丢弃到行尾。
- 单个参数最多 `CLI_TOKEN_MAX - 1`（默认 31）个字符，超长的参数被丢弃。
- 最多保存 `CLI_MAX_PARAMS`（默认 8）个参数，多余参数被静默忽略。
- 解析使用 `atof()`：`"abc"` 会得到 0，`"12abc"` 会得到 12，非法输入不会报错。
- `nan`、`inf` 可能被当作合法浮点数接受，命令内的范围检查请使用 `!((v >= min) && (v <= max))` 形式，才能真正拦住 NaN。
- 十六进制使用 `unsigned long` 累加，超长输入会回绕后再转成 `double`。

---

## 9. 输出与 printf

### 9.1 后端选择

`cli_config.h`：

```c
#define CLI_USE_STDIO_PRINTF 0
```

| 值 | 行为 |
|---|---|
| `0`（默认） | 使用内建格式化实现，不依赖 libc `printf` |
| `1` | 转发到标准库 `vprintf()` |

也可以在编译命令中覆盖，无需修改头文件：

```text
-DCLI_USE_STDIO_PRINTF=1
```

两种后端输出路径不同：

- 内建实现调用 `cli_io_putc()`；
- `vprintf()` 走 stdout，最终由 BSP 的 `fputc()` 输出。

如果使用自定义串口并选择 stdio 后端，必须同时提供 `fputc()` / `_write()`，否则输出不会到你期望的串口。

### 9.2 支持的格式

| 格式 | 支持 | 说明 |
|---|---|---|
| `%c` | 是 | |
| `%s` | 是 | 空指针输出 `(null)` |
| `%d` | 是 | 参数类型为 `int` |
| `%u` | 是 | 无符号值请使用它，支持宽度 |
| `%x` / `%X` | 是 | 小写/大写，支持宽度，如 `%08x` |
| `%f` | 是 | 默认 6 位小数，支持 `%.2f` |
| `%%` | 是 | 输出 `%` |
| `%ld` / `%llu` | 否 | 需要时切换到 stdio 后端 |
| `%p` | 否 | 同上 |

注意：

- `%d` 不会做宽度补零；宽度只对 `%u`、`%x`、`%X` 生效。
- 不要用 `%d` 打印 `uint32_t`，请用 `%u`。
- `%f` 不处理 NaN/Inf，避免把这类值送进 `cli_printf`。
- `cli_itoa()` 返回静态缓冲区，同一条表达式里不要调用两次。

---

## 10. 移植到其他平台

修改 `cli_io.c` 顶部的四个端口函数即可：

```c
int  cli_io_init(unsigned long baud_rate);
void cli_io_putc(char c);
int  cli_io_getc(void);
void cli_io_puts(const char *s);
```

要点：

- `cli_io_getc()` 必须返回 `int`，范围 `0..255`，无数据或结束时返回 `CLI_EOF`（`-1`）。不要用 `char` 作为返回类型，否则在 `char` 无符号的平台（如 ARM）上 EOF 判断会失效。
- 当前实现是阻塞读；没有超时、没有非阻塞接口。
- `cli_io_init()` 默认是空实现，直接返回 `CLI_OK`。
- 默认端口使用 `putchar()` / `getchar()`，依赖 BSP 的 `fputc()` / `inbyte()`。

---

## 11. 限制与注意事项

| 项 | 值 / 行为 |
|---|---|
| 单行长度 | `CLI_LINE_MAX - 1`，默认 63 字符 |
| 单参数长度 | `CLI_TOKEN_MAX - 1`，默认 31 字符 |
| 参数个数 | `CLI_MAX_PARAMS`，默认 8，超出静默丢弃 |
| 输入编码 | 按 ASCII 处理 |
| 读取方式 | 阻塞，无超时 |
| CRLF | `\r` 结束本行，遗留的 `\n` 会产生一次空行读取，被 `cli_run()` 跳过 |
| 超长行 | 截断后按前缀执行 |
| 线程安全 | 无；`cli_active_command`、`cli_itoa` 的缓冲均为静态 |
| 命令注册 | 静态表，不支持运行时注册 |

补充：当 `max_param_count` 等于 `CLI_MAX_PARAMS` 时，用户多输入的参数会在解析阶段被丢弃，数量校验无法发现。当前模板的最大值是 1，不受影响。

---

## 12. 常见问题

| 症状 | 可能原因 |
|---|---|
| 所有命令都报 `Invalid command!` | 表项 `name` 缺少 `!` / `?` / `=`；或命令没有加入命令表 |
| 正确的输入报 `Invalid parameter!` | 参数个数与 `min/max` 不符；或命令内部取值校验未通过 |
| 每条命令后多一次空行 | 串口终端发送的是 CRLF，`\n` 被当作一次空行 |
| 命令被截断 | 输入超过 63 字符，或单个参数超过 31 字符 |
| 没有任何输出 | 没有调用 `cli_init()`；端口未实现 `cli_io_putc()`；stdio 后端缺少 `fputc()` |
| 切换 printf 后端后格式异常 | 使用了不支持的格式符（如 `%ld`、`%p`） |
| `example_set=nan` 没有报错 | 命令缺少范围校验；参考 5.1 节的 NaN 安全写法 |

---

## 13. 验证

### 13.1 主机侧语法检查

```text
gcc -std=c99 -Wall -Wextra -Wpedantic -I. -fsyntax-only \
    cli_io.c cli_parser.c cli_command.c cli_app_cmds.c
```

### 13.2 完整编译

在自己的 `main.c` 所在目录执行：

```text
gcc -std=c99 -Wall -Wextra -Wpedantic -I. \
    cli_io.c cli_parser.c cli_command.c cli_app_cmds.c main.c -o cli_demo
```

### 13.3 手工冒烟清单

目前没有自动化测试，建议至少覆盖：

```text
help?
example_get?
example_get_addr?
example_get_addr? 0
example_get_addr? 1
example_get_addr? nan
example_set=123
example_set=
example_set=-1
example_set=4294967295
example_set=4294967296
example_set=nan
example_do!
example_do! 1
unknown_cmd
（连续输入三行有效命令）
exit!
```

预期：

- 连续输入的命令逐条执行，不会丢行；
- 未知命令只打印一次 `Invalid command!`；
- `example_set=`、`example_set=-1`、`example_set=4294967296`、`example_set=nan` 都打印 `Invalid parameter!` 和命令说明；
- `example_do! 1` 正常执行并忽略 `1`；
- `exit!` 打印 `Exiting CLI.` 后 `cli_run()` 返回。

---

## 14. 当前状态

- 定位：原型模板，从既有工程中剥离。
- 文档：本文件为第一版。
- 测试：尚无仓库内的自动化用例，只有第 13 节的手工清单。
- 未实现：行内编辑（回显/退格）、提示符、非阻塞读取、运行时命令注册。
- `CLI_EXEC_PARSEFAIL` 当前与 `CLI_EXEC_CONTINUE` 同值，仅作为语义占位，详见 7.4 节。
