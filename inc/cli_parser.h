/******************************************************************************
 *   @file   cli_parser.h
 *   @brief  Line reader and command parser for the generic CLI.
 ******************************************************************************/

#ifndef CLI_PARSER_H
#define CLI_PARSER_H

#include <stddef.h>
#include <stdint.h>

#define CLI_READ_EOF ((size_t)-1)

/* Result of cli_parse_line(). */
#define CLI_CMD_UNKNOWN   (-1)
#define CLI_CMD_DO        (0)
#define CLI_CMD_READ      (1)
#define CLI_CMD_WRITE     (2)

/* Reads one line into line (at most size - 1 characters).
 * The trailing '\r'/'\n' is consumed but not stored, and the result is always
 * NUL-terminated. Returns the number of characters stored.
 * If the line is ended with '\r\n', the '\n' will be treated as empty line.
 * Extra characters beyond size - 1 are discarded until the end of the line. */
size_t cli_read_line(char *line, size_t size);

/* Matches input_line against command_pattern and parses its parameters.
 *
 * A command pattern ends with '!' (do), '?' (read) or '=' (write), e.g.
 * "help?", "tx_lo_freq=", "exit!". Parameters are whitespace separated and
 * may be decimal, negative or hexadecimal (0x...).
 *
 * Returns CLI_CMD_DO / CLI_CMD_READ / CLI_CMD_WRITE on a match, or
 * CLI_CMD_UNKNOWN when the line does not match. At most CLI_MAX_PARAMS
 * parameters are stored in params; param_count receives the stored count. */
int cli_parse_line(const char *input_line,
                   const char *command_pattern,
                   double *params,
                   uint8_t *param_count);

#endif /* CLI_PARSER_H */
