/* utils.h - safe input helpers and string helpers used by all modules */
#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/* Reads one line safely (no buffer overflow). Strips the newline.
   Exits the program cleanly if input is closed (EOF). */
void   read_line(char *buf, size_t size);

/* Keeps asking until the user enters a whole number in [min, max]. */
int    read_int_in_range(const char *prompt, int min, int max);

/* Keeps asking until the user enters a positive number (digits and one '.').
   If default_value > 0, pressing Enter returns it; otherwise input is required. */
double read_positive_double(const char *prompt, double default_value);

/* Asks a y/n question. Returns 1 for yes, 0 for no. */
int    read_yes_no(const char *prompt);

/* Reads a password without showing it on screen (when run in a terminal). */
void   read_password(const char *prompt, char *buf, size_t size);

void   pause_for_enter(void);

/* String helpers */
void   str_trim(char *s);                       /* removes leading/trailing spaces */
int    str_equals_ignore_case(const char *a, const char *b);
int    str_contains_ignore_case(const char *text, const char *keyword);

#endif
