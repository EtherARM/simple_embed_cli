/******************************************************************************
 *   @file   cli_config.h
 *   @brief  Compile-time configuration for the generic CLI framework.
 ******************************************************************************/

#ifndef CLI_CONFIG_H
#define CLI_CONFIG_H

/* Maximum number of parameters accepted by a single command. */
#ifndef CLI_MAX_PARAMS
#define CLI_MAX_PARAMS 8
#endif

/* Maximum length of an input line, including the terminating '\0'. */
#ifndef CLI_LINE_MAX
#define CLI_LINE_MAX 64
#endif

/* Maximum length of one parameter token, including the terminating '\0'. */
#ifndef CLI_TOKEN_MAX
#define CLI_TOKEN_MAX 32
#endif

/* Default UART baud rate used by cli_init(). */
#ifndef CLI_BAUD_RATE
#define CLI_BAUD_RATE 115200UL
#endif

/* printf backend selection:
 *   0 - built-in cli_printf implementation (small, no libc printf)
 *   1 - forward cli_printf to the C library's vprintf()
 * Override with -DCLI_USE_STDIO_PRINTF=1 or by editing this file. */
#ifndef CLI_USE_STDIO_PRINTF
#define CLI_USE_STDIO_PRINTF 0
#endif

#endif /* CLI_CONFIG_H */
