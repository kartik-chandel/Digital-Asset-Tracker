/* market.h - price source: sample data now, CoinGecko in Phase 5 */
#ifndef MARKET_H
#define MARKET_H

#include "common.h"

/* Current INR price for symbol, or -1.0 if unknown. */
double market_get_price(const AppState *app, const char *symbol);
void   market_print_prices(const AppState *app);

/* Phase 5: fetch live prices with libcurl + cJSON. Returns 1 on success. */
int    market_refresh_from_api(AppState *app);

#endif
