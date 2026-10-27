/******************************************************************************
 *   @file   cli_command.c
 *   @brief  Command table, dispatcher and main loop for the generic CLI.
 ******************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include "cli_command.h"
#include "cli_config.h"
#include "cli_io.h"
#include "cli_parser.h"

#define CLI_NO_ACTIVE_COMMAND ((size_t)-1)

static size_t cli_active_command = CLI_NO_ACTIVE_COMMAND;

void cli_print_invalid_param(void)
{
    cli_printf("Invalid parameter!\n");

    if (cli_active_command < g_cli_command_count)
    {
        const cli_command_t *command = &g_cli_command_table[cli_active_command];

        cli_printf("%s  - %s\n", command->name, command->help);
        if ((command->usage != NULL) && (command->usage[0] != '\0'))
        {
            cli_printf("Example: %s\n", command->usage);
        }
    }
}

int cli_execute_line(const char *line)
{
    double  params[CLI_MAX_PARAMS] = { 0 };
    uint8_t param_count = 0;
    size_t  index;
    int parse_result = CLI_CMD_UNKNOWN;

    if (line == NULL)
    {
        return CLI_EXEC_CONTINUE;
    }

    for (index = 0; index < g_cli_command_count; index++)
    {
        param_count = 0;
        parse_result = cli_parse_line(line,g_cli_command_table[index].name,
                           params,&param_count);
        if (parse_result == CLI_CMD_UNKNOWN)
        {
            // command parse mismatch, use next command in table as reference.
            continue;
        }
        if (parse_result == CLI_CMD_DO)
        {
            // DO commands are not parameter sensitive.
            cli_active_command = index;
            return g_cli_command_table[index].handler(params, param_count);
        }
        if ((parse_result == CLI_CMD_WRITE) || (parse_result == CLI_CMD_READ))
        {
            // WRITE commands and address sensitive READ commands are parameter sensitive.
            // Check the number of parameters sent in here. 
            cli_active_command = index;
            if ((param_count > g_cli_command_table[index].max_param_count) |
                (param_count < g_cli_command_table[index].min_param_count))
            {
                cli_print_invalid_param();
                return CLI_EXEC_PARSEFAIL;
            }
            else
            {
                return g_cli_command_table[index].handler(params, param_count);
            }
        }
    }

    // Command input not found in command table.
    cli_printf("Invalid command!\n");
    return CLI_EXEC_PARSEFAIL;
}

void cli_run(void)
{
    char line[CLI_LINE_MAX];
    int  result = CLI_EXEC_CONTINUE;
    size_t n;

    while (result != CLI_EXEC_EXIT)
    {
        n = cli_read_line(line, sizeof(line));
        if (n == CLI_READ_EOF)
        {
            return;
        }
        if (n == 0)
        {
            continue;
        }

        result = cli_execute_line(line);
    }
}

int cli_cmd_help(double *params, uint8_t param_count)
{
    size_t index;

    (void)params;
    (void)param_count;

    cli_printf("Available commands:\n");
    for (index = 0; index < g_cli_command_count; index++)
    {
        cli_printf("%s  - %s\n",
                   g_cli_command_table[index].name,
                   g_cli_command_table[index].help);
    }

    return CLI_EXEC_CONTINUE;
}

int cli_cmd_exit(double *params, uint8_t param_count)
{
    (void)params;
    (void)param_count;

    cli_printf("Exiting CLI.\n");
    return CLI_EXEC_EXIT;
}
