/* utils.c - safe input helpers and string helpers */
#define _POSIX_C_SOURCE 200809L   /* for isatty/termios under -std=c11 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

#ifdef _WIN32
  #include <conio.h>
#else
  #include <termios.h>
  #include <unistd.h>
#endif

#define MAX_INPUT_VALUE 1e12   /* upper limit for quantities and prices */

/* After fgets: remove the newline, or discard the rest of an over-long line. */
static void strip_newline_or_flush(char *buf)
{
    size_t len = strlen(buf);

    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) { }
    }
}

void read_line(char *buf, size_t size)
{
    if (fgets(buf, (int)size, stdin) == NULL) {
        printf("\nInput closed. Exiting.\n");
        exit(0);
    }
    strip_newline_or_flush(buf);
}

int read_int_in_range(const char *prompt, int min, int max)
{
    char  line[64];
    char *end;
    long  value;

    while (1) {
        printf("%s", prompt);
        read_line(line, sizeof line);

        if (line[0] == '\0') {
            printf("Please enter a number.\n");
            continue;
        }

        value = strtol(line, &end, 10);
        while (*end == ' ') end++;            /* allow trailing spaces */

        if (*end != '\0' || value < min || value > max) {
            printf("Invalid choice. Enter a number between %d and %d.\n", min, max);
            continue;
        }
        return (int)value;
    }
}

/* Accepts only digits with at most one '.', and at least one digit. */
static int is_plain_number(const char *s)
{
    int digits = 0, dots = 0;

    for (; *s; s++) {
        if (isdigit((unsigned char)*s))      digits++;
        else if (*s == '.' && dots == 0)     dots++;
        else                                 return 0;
    }
    return digits > 0;
}

double read_positive_double(const char *prompt, double default_value)
{
    char   line[64];
    double value;

    while (1) {
        printf("%s", prompt);
        read_line(line, sizeof line);
        str_trim(line);

        if (line[0] == '\0') {
            if (default_value > 0.0) return default_value;
            printf("This value is required.\n");
            continue;
        }
        if (!is_plain_number(line)) {
            printf("Invalid input. Enter a positive number like 0.5 or 1500.75 "
                   "(no signs, letters or commas).\n");
            continue;
        }

        value = strtod(line, NULL);
        if (value <= 0.0) {
            printf("Value must be greater than zero.\n");
            continue;
        }
        if (value > MAX_INPUT_VALUE) {
            printf("Value is too large.\n");
            continue;
        }
        return value;
    }
}

int read_yes_no(const char *prompt)
{
    char line[16];

    while (1) {
        printf("%s", prompt);
        read_line(line, sizeof line);
        str_trim(line);

        if (str_equals_ignore_case(line, "y") || str_equals_ignore_case(line, "yes")) return 1;
        if (str_equals_ignore_case(line, "n") || str_equals_ignore_case(line, "no"))  return 0;
        printf("Please answer y or n.\n");
    }
}

#ifdef _WIN32
void read_password(const char *prompt, char *buf, size_t size)
{
    size_t len = 0;
    int    ch;

    printf("%s", prompt);
    fflush(stdout);

    while ((ch = _getch()) != '\r' && ch != '\n') {
        if (ch == '\b') {
            if (len > 0) len--;
        } else if (len + 1 < size && ch >= 32) {
            buf[len++] = (char)ch;
        }
    }
    buf[len] = '\0';
    printf("\n");
}
#else
void read_password(const char *prompt, char *buf, size_t size)
{
    struct termios old_settings, new_settings;
    int   hide = isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &old_settings) == 0;
    char *result;

    printf("%s", prompt);
    fflush(stdout);

    if (hide) {                              /* turn terminal echo off */
        new_settings = old_settings;
        new_settings.c_lflag &= ~(tcflag_t)ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
    }

    result = fgets(buf, (int)size, stdin);

    if (hide) {                              /* always restore echo */
        tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
    }
    printf("\n");

    if (result == NULL) {
        printf("Input closed. Exiting.\n");
        exit(0);
    }
    strip_newline_or_flush(buf);
}
#endif

void pause_for_enter(void)
{
    char tmp[8];
    printf("\nPress Enter to continue...");
    read_line(tmp, sizeof tmp);
}

void str_trim(char *s)
{
    size_t len   = strlen(s);
    size_t start = 0;

    while (s[start] && isspace((unsigned char)s[start])) start++;
    while (len > start && isspace((unsigned char)s[len - 1])) len--;

    memmove(s, s + start, len - start);
    s[len - start] = '\0';
}

int str_equals_ignore_case(const char *a, const char *b)
{
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == *b;
}

int str_contains_ignore_case(const char *text, const char *keyword)
{
    size_t tl = strlen(text);
    size_t kl = strlen(keyword);

    if (kl == 0) return 1;
    if (kl > tl) return 0;

    for (size_t i = 0; i + kl <= tl; i++) {
        size_t j = 0;
        while (j < kl &&
               tolower((unsigned char)text[i + j]) == tolower((unsigned char)keyword[j])) {
            j++;
        }
        if (j == kl) return 1;
    }
    return 0;
}
