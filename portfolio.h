/* portfolio.h - holdings of the logged-in user */
#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include "common.h"

/* Quantities below this are treated as zero (floating-point safety). */
#define QTY_EPSILON 1e-9

/* Index of this user's holding for symbol, or -1. */
int    portfolio_find_holding(const AppState *app, int user_id, const char *symbol);
int    portfolio_user_holding_count(const AppState *app, int user_id);
void   portfolio_view(const AppState *app);
int    portfolio_add_holding(AppState *app);          /* menu option 2 */
void   portfolio_remove_if_empty(AppState *app, int holding_index);
double portfolio_total_value(const AppState *app, int user_id);

#endif
