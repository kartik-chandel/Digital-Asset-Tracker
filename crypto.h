/* crypto.h - cryptocurrency database and search */
#ifndef CRYPTO_H
#define CRYPTO_H

#include "common.h"

/* Fills app->cryptos with the built-in sample coins (INR prices). */
void crypto_load_samples(AppState *app);

/* Returns the index of the coin with this symbol, or -1 if not found. */
int  crypto_find_by_symbol(const AppState *app, const char *symbol);

/* Finds coins whose name or symbol contains keyword (case-insensitive).
   Stores matching indexes in results[] (up to max_results).
   Returns how many matched. */
int  crypto_search(const AppState *app, const char *keyword,
                   int results[], int max_results);

/* Asks the user for a coin symbol. Returns 1 and sets *index_out if the coin
   exists. Returns 0 (after printing why) if cancelled or not found. */
int  crypto_prompt_symbol(const AppState *app, const char *prompt, int *index_out);

/* Prints a table of the coins at the given indexes. */
void crypto_print_matches(const AppState *app, const int indexes[], int count);

/* Prints a table of every coin. */
void crypto_print_all(const AppState *app);

#endif
