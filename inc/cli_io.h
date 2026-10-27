/******************************************************************************
 *   @file   cli_io.h
 *   @brief  UART abstraction and formatted output for the generic CLI.
 ******************************************************************************/

#ifndef CLI_IO_H
#define CLI_IO_H

#include <stddef.h>

/* Generic status codes. */
#define CLI_OK   (0)
#define CLI_ERR  (-1)
#define CLI_EOF  (-1)

/* Initializes the serial port used by the CLI. */
int cli_io_init(unsigned long baud_rate);

/* Writes one character to the serial port. */
void cli_io_putc(char c);

/* Reads one character from the serial port (blocking). */
int cli_io_getc(void);

/* Writes a NUL-terminated string to the serial port. */
void cli_io_puts(const char *s);

/* Prints formatted data. Supports %c, %s, %d, %u, %x, %X, %f and %%.
 * Field width is supported for %u/%x/%X, precision for %f.
 * The implementation is selected at compile time by CLI_USE_STDIO_PRINTF
 * (0 = built-in formatter, 1 = C library vprintf). */
void cli_printf(const char *fmt, ...);

/* Converts a signed integer to a string. radix must be in [2, 16].
 * The returned buffer is static, so the result is valid until the next call. */
char *cli_itoa(long value, unsigned char radix);

/* Initializes the CLI I/O layer (currently just the serial port). */
int cli_init(unsigned long baud_rate);

#endif /* CLI_IO_H */
