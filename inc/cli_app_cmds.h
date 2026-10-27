/******************************************************************************
 *   @file   cli_app_cmds.h
 *   @brief  CLI application command handlers for the generic CLI.
 ******************************************************************************/

#ifndef CLI_APP_CMDS_H
#define CLI_APP_CMDS_H

#include <stdint.h>

/* Example commands. */
int cmd_example_get(double *params, uint8_t param_count);
int cmd_example_get_addr(double *params, uint8_t param_count);
int cmd_example_set(double *params, uint8_t param_count);
int cmd_example_do(double *params, uint8_t param_count);

#endif /* CLI_APP_CMDS_H */
