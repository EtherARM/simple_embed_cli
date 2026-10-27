/******************************************************************************
 *   @file   cli_parser.c
 *   @brief  Line reader and command parser for the generic CLI.
 ******************************************************************************/

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "cli_config.h"
#include "cli_io.h"
#include "cli_parser.h"
#include "cli_command.h"

static int cli_is_separator(char c)
{
    return (c == ' ') || (c == '\t');
}

static int cli_is_hex_digit(char c)
{
    return ((c >= '0') && (c <= '9')) ||
           ((c >= 'a') && (c <= 'f')) ||
           ((c >= 'A') && (c <= 'F'));
}

static unsigned char cli_hex_value(char c)
{
    if ((c >= '0') && (c <= '9'))
    {
        return (unsigned char)(c - '0');
    }
    if ((c >= 'a') && (c <= 'f'))
    {
        return (unsigned char)(c - 'a' + 10);
    }
    return (unsigned char)(c - 'A' + 10);
}

static double cli_parse_value(const char *token)
{
    if ((token[0] == '0') && ((token[1] == 'x') || (token[1] == 'X')))
    {
        unsigned long value = 0;
        const char *p = token + 2;

        while (cli_is_hex_digit(*p))
        {
            value = (value * 16UL) + cli_hex_value(*p);
            p++;
        }
        return (double)value;
    }

    return atof(token);
}

size_t cli_read_line(char *line, size_t size)
{
    size_t count = 0;
    int c = 0;

    if ((line == NULL) || (size == 0))
    {
        return 0;
    }

    for (;;)
    {
        c = cli_io_getc();
        if(c == CLI_EOF)
        {
            line[count] = '\0';
            return (count > 0) ? count : CLI_READ_EOF;
        }
        if ((c == '\n') || (c == '\r'))
        {
            break;
        }

        if (count + 1 < size)
        {
            line[count++] = c;
        }
        /* else: silently discard the rest of an overlong line */
    }

    line[count] = '\0';
    return count;
}

int cli_parse_line(const char *input_line,
                   const char *command_pattern,
                   double *params,
                   uint8_t *param_count)
{
    size_t index = 0;
    const char *p;
    char terminator;
    int command_type;

    if ((input_line == NULL) || (command_pattern == NULL) ||
        (params == NULL) || (param_count == NULL))
    {
        return CLI_CMD_UNKNOWN;
    }

    /* Compare the literal prefix, up to the command terminator. */
    while ((command_pattern[index] != '\0') &&
           (command_pattern[index] != '!') &&
           (command_pattern[index] != '?') &&
           (command_pattern[index] != '='))
    {
        if (input_line[index] != command_pattern[index])
        {
            return CLI_CMD_UNKNOWN;
        }
        index++;
    }

    terminator = command_pattern[index];
    if (terminator == '\0')
    {
        return CLI_CMD_UNKNOWN;
    }

    if (input_line[index] != terminator)
    {
        return CLI_CMD_UNKNOWN;
    }

    switch (terminator)
    {
    case '!':
        command_type = CLI_CMD_DO;
        break;
    case '?':
        command_type = CLI_CMD_READ;
        break;
    case '=':
        command_type = CLI_CMD_WRITE;
        break;
    default:
        return CLI_CMD_UNKNOWN;
    }

    *param_count = 0;

    /* A do-command takes no parameters. */
    if (command_type == CLI_CMD_DO)
    {
        return command_type;
    }

    p = input_line + index + 1;
    while (*p != '\0')
    {
        char token[CLI_TOKEN_MAX];
        size_t length = 0;
        int truncated = 0;

        while (cli_is_separator(*p))
        {
            p++;
        }
        if (*p == '\0')
        {
            break;
        }

        while ((*p != '\0') && !cli_is_separator(*p))
        {
            if (length + 1 < sizeof(token))
            {
                token[length++] = *p;
            }
            else
            {
                truncated = 1;
            }
            p++;
        }
        token[length] = '\0';

        if ((*param_count < CLI_MAX_PARAMS) && !truncated)
        {
            params[*param_count] = cli_parse_value(token);
            (*param_count)++;
        }
    }

    return command_type;
}
