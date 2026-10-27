/******************************************************************************
 *   @file   cli_io.c
 *   @brief  IO abstraction and formatted output for the generic CLI.
 ******************************************************************************/

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#include "cli_config.h"
#include "cli_io.h"

/* ==========================================================================
 * Platform port layer
 *
 * These four functions are the only place that touches the serial hardware.
 * The default implementation uses stdio, which the Xilinx standalone BSP
 * routes to the PS UART through fputc()/inbyte(). Replace this block when
 * porting the framework to another platform.
 * ========================================================================== */

int cli_io_init(unsigned long baud_rate)
{
    (void)baud_rate;
    return CLI_OK;
}

void cli_io_putc(char c)
{
    (void)putchar((int)c);
}

int cli_io_getc(void)
{
    return getchar();
}

void cli_io_puts(const char *s)
{
    if (s == NULL)
    {
        return;
    }

    while (*s != '\0')
    {
        cli_io_putc(*s++);
    }
}

/* ==========================================================================
 * Formatted output
 * ========================================================================== */

char *cli_itoa(long value, unsigned char radix)
{
    static char buffer[66];
    char *cursor = &buffer[sizeof(buffer) - 1];
    unsigned long magnitude;
    char is_negative = 0;

    if (radix < 2 || radix > 16)
    {
        radix = 10;
    }

    *cursor = '\0';

    if (value < 0 && radix == 10)
    {
        is_negative = 1;
        magnitude = (unsigned long)(-(value + 1)) + 1UL;
    }
    else
    {
        magnitude = (unsigned long)value;
    }

    do
    {
        *--cursor = "0123456789ABCDEF"[magnitude % radix];
        magnitude /= radix;
    } while (magnitude != 0);

    if (is_negative)
    {
        *--cursor = '-';
    }

    return cursor;
}

#if !CLI_USE_STDIO_PRINTF
static void cli_print_unsigned(unsigned long value,
                               unsigned char radix,
                               unsigned char min_digits,
                               int uppercase)
{
    const char *digits = uppercase ? "0123456789ABCDEF"
                                   : "0123456789abcdef";
    char buffer[66];
    unsigned char count = 0;

    do
    {
        buffer[count++] = digits[value % radix];
        value /= radix;
    } while (value != 0);

    while (count < min_digits && count < sizeof(buffer))
    {
        buffer[count++] = '0';
    }

    while (count > 0)
    {
        cli_io_putc(buffer[--count]);
    }
}
#endif /* !CLI_USE_STDIO_PRINTF */

void cli_printf(const char *fmt, ...)
{
    va_list args;
#if !CLI_USE_STDIO_PRINTF
    const char *p;
#endif

    if (fmt == NULL)
    {
        return;
    }

    va_start(args, fmt);

#if CLI_USE_STDIO_PRINTF
    /* Full C library formatting; output still goes through stdout/fputc. */
    (void)vprintf(fmt, args);
#else
    for (p = fmt; *p != '\0'; p++)
    {
        unsigned int field_width = 0;
        unsigned int precision = 6;

        if (*p != '%')
        {
            cli_io_putc(*p);
            continue;
        }

        p++;
        if (*p == '%')
        {
            cli_io_putc('%');
            continue;
        }

        while (*p >= '0' && *p <= '9')
        {
            field_width = field_width * 10 + (unsigned int)(*p - '0');
            p++;
        }
        if (field_width > 32)
        {
            field_width = 32;
        }

        if (*p == '.')
        {
            p++;
            precision = 0;
            while (*p >= '0' && *p <= '9')
            {
                precision = precision * 10 + (unsigned int)(*p - '0');
                p++;
            }
            if (precision > 9)
            {
                precision = 9;
            }
        }

        switch (*p)
        {
        case 'c':
            cli_io_putc((char)va_arg(args, int));
            break;

        case 's':
        {
            const char *s = va_arg(args, const char *);
            cli_io_puts((s != NULL) ? s : "(null)");
            break;
        }

        case 'd':
            cli_io_puts(cli_itoa((long)va_arg(args, int), 10));
            break;

        case 'u':
            cli_print_unsigned((unsigned long)va_arg(args, unsigned int),
                               10,
                               (unsigned char)field_width,
                               0);
            break;

        case 'x':
        case 'X':
            cli_print_unsigned((unsigned long)va_arg(args, unsigned int),
                               16,
                               (unsigned char)field_width,
                               (*p == 'X'));
            break;

        case 'f':
        {
            double value = va_arg(args, double);
            unsigned int decimals = precision;
            unsigned long scale = 1;
            unsigned long integer_part;
            unsigned long fraction;
            unsigned int i;

            for (i = 0; i < decimals; i++)
            {
                scale *= 10;
            }

            if (value < 0)
            {
                cli_io_putc('-');
                value = -value;
            }

            integer_part = (unsigned long)value;
            fraction = (unsigned long)((value - (double)integer_part) *
                                       (double)scale + 0.5);
            if (fraction >= scale)
            {
                integer_part++;
                fraction -= scale;
            }

            cli_print_unsigned(integer_part, 10, 0, 0);
            if (decimals > 0)
            {
                cli_io_putc('.');
                cli_print_unsigned(fraction, 10, (unsigned char)decimals, 0);
            }
            break;
        }

        case '\0':
            cli_io_putc('%');
            p--;
            break;

        default:
            cli_io_putc('%');
            cli_io_putc(*p);
            break;
        }
    }
#endif /* CLI_USE_STDIO_PRINTF */

    va_end(args);
}

int cli_init(unsigned long baud_rate)
{
    return cli_io_init(baud_rate);
}
