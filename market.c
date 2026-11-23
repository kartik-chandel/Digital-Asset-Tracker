/* market.c - price source. Sample data for now; CoinGecko arrives in Phase 5. */
#include <stdio.h>
#include "market.h"
#include "crypto.h"

double market_get_price(const AppState *app, const char *symbol)
{
    int idx = crypto_find_by_symbol(app, symbol);

    if (idx < 0) return -1.0;
    return app->cryptos[idx].price_inr;
}

void market_print_prices(const AppState *app)
{
    printf("\nMarket prices in INR (sample data until live prices arrive in Phase 5)");
    crypto_print_all(app);
}

int market_refresh_from_api(AppState *app)
{
    (void)app;
    printf("Live prices are not available yet (Phase 5).\n");
    return 0;
}
