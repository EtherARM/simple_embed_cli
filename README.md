# Simple Embed CLI Framework
A very simple bare metal CLI framework for embedded development and hardware testing.

## Features
- Static command table
- No dynamic memory allocation
- Supports three types of commands: DO, READ and WRITE
- Supports signed decimals and hexadecimals as parameters
- In-framework parameter number checking
- Compile-time selection of print backends: built-in / `vprintf`
- Designated IO port layer
- Fixed buffer and truncation of inputs
- Blocking operation
- Tested: Vitis IDE

## File Structure
| File | Function |
| --- | --- |
| `cli_config.h` | Configs |
| `cli_io.h` / `cli_io.c` | IO ports and Formatted Output |
| `cli_parser.h` / `cli_parser.c` | Input Parser |
| `cli_command.h` / `cli_command.c` | Dispatcher and Main Loop |
| `cli_app_cmds.h` / `cli_app_cmds.c` | Application-specific Command Table and Handlers |

Application-specific features are implemented only in `cli_app_cmds.h` / `cli_app_cmds.c`.

## Command Grammar
```
[name][terminator][params]
```
Three characters are accepted as terminators.
| Ending | Type | Example | Parameter Sensitivity |
|---|---|---|---|
| `!` | DO | `exit!` | Not sensitive |
| `?` | READ | `example_get?` | Sensitive |
| `=` | WRITE | `example_set=123` | Sensitive |

Parameters are separated with space or tab.

## Integration
1. Clone or download this repository, copy the sources and headers into your project. You may re-arrange the directory layout as you want.
2. Modify these functions inside `cli_io.c` to fit your platform:
    ```c
    int  cli_io_init(unsigned long baud_rate);
    void cli_io_putc(char c);
    int  cli_io_getc(void);
    void cli_io_puts(const char *s);
    ```

## Config
Macro configs are placed inside `cli_config.h`.

## Usage
```C
/* Your code */
#include "cli_io.h"
#include "cli_command.h"
#include "cli_config.h"

/* Your code */

void main()
{
    /* Your code */
    cli_init(CLI_BAUD_RATE);
    cli_run();
    /* Your code */
}
```

## Testing
A manual smoke test:
```terminal
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
[valid commands]
exit!
```
Expected output：
- Command is executed successively.
- Only one `Invalid command!` is output for each invalid command.
- `example_set=`,`example_set=-1`,`example_set=4294967296`,`example_set=nan` generates `Invalid parameter!`.
- `example_do! 1` executes and ignores `1`.
- `exit!` outputs `Exiting CLI.`, and `cli_run()` returns.

## To-Do

- [ ] Documentation on adding user commands
- [ ] In-file comments
- [ ] Further testing plans
- [ ] Example project

## License
MIT License