/* transaction.h - BUY / SELL flow and history */
#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <stddef.h>
#include "common.h"

/* Interactive menu flows (ask the user, then call the record functions). */
int  transaction_buy(AppState *app);
int  transaction_sell(AppState *app);

/* Core logic without prompts. Return 1 on success, 0 on failure
   (a message is printed on failure). realized_out may be NULL. */
int  transaction_record_buy(AppState *app, const char *symbol,
                            double quantity, double price);
int  transaction_record_sell(AppState *app, const char *symbol,
                             double quantity, double price, double *realized_out);

void transaction_print_history(const AppState *app);
void transaction_get_timestamp(char *buf, size_t size);

#endif
