/******************************************************************************
 *   @file   cli_app_cmds.c
 *   @brief  CLI application command handlers and command table.
 *
 * This is the application layer. Call application drivers here, the CLI
 * framework does not touch the specific hardware or software.
 ******************************************************************************/

#include "cli_app_cmds.h"

#include "cli_command.h"
#include "cli_io.h"

/***********user includes***********/

/***********variables***********/

static uint32_t cli_example_value = 0;

/***********user extern variables***********/


/* ==========================================================================
 * Command table
 * ========================================================================== */

const cli_command_t g_cli_command_table[] =
{
    {"help?",               "Displays all available commands.",         "",0,0, cli_cmd_help},
    {"example_get?",        "Example read command. Reads an internal uint32_t variable.",    "",0,0, cmd_example_get},
    {"example_get_addr?",        "Example address sensitive read command. Reads an internal uint32_t variable if parameter is 0.",    "",1,1, cmd_example_get_addr},
    {"example_set=",        "Example write command. Writes an internal uint32_t variable.",   "",1,1, cmd_example_set},
    {"example_do!",         "Example do command.",                      "",0,0, cmd_example_do},

    // User commands

    // User commands end here

    {"exit!",               "Exits the CLI application.",               "",0,0, cli_cmd_exit}
};

const size_t g_cli_command_count =
    sizeof(g_cli_command_table) / sizeof(g_cli_command_table[0]);

int cmd_example_get(double *params, uint8_t param_count)
{
    uint32_t get_value;

    (void)params;
    (void)param_count;
    get_value = cli_example_value;
    cli_printf("example_get=%u\n", get_value);

    return CLI_EXEC_CONTINUE;
}

int cmd_example_get_addr(double *params, uint8_t param_count)
{
    uint32_t get_value;
    uint32_t addr;

    (void)param_count;
    if (!((params[0] >= 0) && (params[0] <= (double)UINT32_MAX)))
    {
        cli_print_invalid_param();
        return CLI_EXEC_CONTINUE;
    }

    addr = (uint32_t)params[0];
    
    if(addr == 0)
    {
        get_value = cli_example_value;
        cli_printf("example_get_addr=%u\n", get_value);
    }
    else
    {
        cli_printf("Not specified example address!\n");
    }

    return CLI_EXEC_CONTINUE;
}

int cmd_example_set(double *params, uint8_t param_count)
{
    uint32_t set_value;

    /* preserved code as reference. does command-individual param number check,
     * current code does param number check inside command dispatcher.
     */
    /*
    if (param_count < 1)
    {
        cli_print_invalid_param();
        return CLI_EXEC_CONTINUE;
    }
    */
   (void)param_count;
    if (!((params[0] >= 0) && (params[0] <= (double)UINT32_MAX)))
    {
        cli_print_invalid_param();
        return CLI_EXEC_CONTINUE;
    }
    set_value = (uint32_t)params[0];
    cli_example_value = set_value;
    cli_printf("example_set=%u\n", set_value);

    return CLI_EXEC_CONTINUE;
}

int cmd_example_do(double *params, uint8_t param_count)
{

    (void)params;
    (void)param_count;

    cli_printf("example_do.\n");
    return CLI_EXEC_CONTINUE;
}
