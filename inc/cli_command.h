/******************************************************************************
 *   @file   cli_command.h
 *   @brief  Command table, dispatcher and main loop for the generic CLI.
 ******************************************************************************/

#ifndef CLI_COMMAND_H
#define CLI_COMMAND_H

#include <stddef.h>
#include <stdint.h>

/* Return values of a command handler. */
#define CLI_EXEC_CONTINUE   (0)   /* command finished, keep the CLI running */
#define CLI_EXEC_EXIT       (1)   /* request cli_run() to leave its loop */

/* Return values of command dispatcher. */
#define CLI_EXEC_PARSEFAIL  (0)   /* parse failed, currently an alias of CLI_EXEC_CONTINUE and does nothing */

/* NOTICE: Command dispatcher (cli_execute_cmd) returns the return value
 * of the command handler by which the command is run.
 * Thus, the command dispatcher and command handlers share the SAME domain
 * of return values. */

/* Command handler: params/param_count come from cli_parse_line(). */
typedef int (*cli_cmd_handler_t)(double *params, uint8_t param_count);
typedef struct
{
    const char       *name;     /* command pattern, e.g. "tx_lo_freq=" */
    const char       *help;     /* one-line description */
    const char       *usage;    /* optional example */
    uint8_t             min_param_count;
    uint8_t             max_param_count;
    cli_cmd_handler_t handler;
} cli_command_t;

/* Command table provided by the application layer (cli_app_cmds.c). */
extern const cli_command_t g_cli_command_table[];
extern const size_t        g_cli_command_count;

/* Matches one line against the table and runs the first matching handler.
 * Returns the handler's return value, or CLI_EXEC_CONTINUE when no command
 * matched. */
int cli_execute_line(const char *line);

/* Reads lines and executes them until a handler returns CLI_EXEC_EXIT. */
void cli_run(void);

/* Built-in "help?" command. */
int cli_cmd_help(double *params, uint8_t param_count);

/* Built-in "exit!" command. */
int cli_cmd_exit(double *params, uint8_t param_count);

/* Prints the usage of the command that is currently being dispatched.
 * Application handlers call this when required parameters are missing. */
void cli_print_invalid_param(void);

#endif /* CLI_COMMAND_H */
