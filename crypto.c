/* crypto.c - cryptocurrency database and search */
#include <stdio.h>
#include "crypto.h"
#include "utils.h"

/* Sample data (INR). Prices are made-up placeholders until
   market.c is connected to CoinGecko in Phase 5. */
typedef struct {
    const char *name;
    const char *symbol;
    const char *api_id;
    double      price_inr;
    double      change_24h;
} SampleCoin;

static const SampleCoin SAMPLES[] = {
    { "Bitcoin",  "BTC",  "bitcoin",     7500000.00,  1.25 },
    { "Ethereum", "ETH",  "ethereum",     280000.00, -0.80 },
    { "Solana",   "SOL",  "solana",        14000.00,  3.10 },
    { "BNB",      "BNB",  "binancecoin",   55000.00,  0.45 },
    { "XRP",      "XRP",  "ripple",          190.00, -1.60 },
    { "Cardano",  "ADA",  "cardano",          55.00,  0.90 },
    { "Dogecoin", "DOGE", "dogecoin",         14.00,  2.35 },
    { "Polkadot", "DOT",  "polkadot",        600.00, -0.25 }
};

void crypto_load_samples(AppState *app)
{
    size_t n = sizeof SAMPLES / sizeof SAMPLES[0];

    app->crypto_count = 0;
    for (size_t i = 0; i < n && i < MAX_CRYPTOS; i++) {
        Crypto *c = &app->cryptos[app->crypto_count];

        c->asset_id = (int)i + 1;
        /* snprintf always null-terminates, so no overflow */
        snprintf(c->name,   sizeof c->name,   "%s", SAMPLES[i].name);
        snprintf(c->symbol, sizeof c->symbol, "%s", SAMPLES[i].symbol);
        snprintf(c->api_id, sizeof c->api_id, "%s", SAMPLES[i].api_id);
        c->price_inr  = SAMPLES[i].price_inr;
        c->change_24h = SAMPLES[i].change_24h;

        app->crypto_count++;
    }
}

int crypto_find_by_symbol(const AppState *app, const char *symbol)
{
    for (int i = 0; i < app->crypto_count; i++) {
        if (str_equals_ignore_case(app->cryptos[i].symbol, symbol)) {
            return i;
        }
    }
    return -1;
}

int crypto_search(const AppState *app, const char *keyword,
                  int results[], int max_results)
{
    int found = 0;

    for (int i = 0; i < app->crypto_count && found < max_results; i++) {
        if (str_contains_ignore_case(app->cryptos[i].name,   keyword) ||
            str_contains_ignore_case(app->cryptos[i].symbol, keyword)) {
            results[found++] = i;
        }
    }
    return found;
}

int crypto_prompt_symbol(const AppState *app, const char *prompt, int *index_out)
{
    char symbol[64];
    int  idx;

    printf("%s", prompt);
    read_line(symbol, sizeof symbol);
    str_trim(symbol);

    if (symbol[0] == '\0') {
        printf("Cancelled.\n");
        return 0;
    }

    idx = crypto_find_by_symbol(app, symbol);
    if (idx < 0) {
        printf("Symbol \"%s\" not found. Try \"Search Cryptocurrency\" first.\n", symbol);
        return 0;
    }

    *index_out = idx;
    return 1;
}

static void print_table_header(void)
{
    printf("\n%-4s %-12s %-7s %16s %10s\n",
           "ID", "Name", "Symbol", "Price (INR)", "24h");
    printf("---------------------------------------------------------\n");
}

static void print_table_row(const Crypto *c)
{
    printf("%-4d %-12s %-7s %16.2f %+9.2f%%\n",
           c->asset_id, c->name, c->symbol, c->price_inr, c->change_24h);
}

void crypto_print_matches(const AppState *app, const int indexes[], int count)
{
    print_table_header();
    for (int i = 0; i < count; i++) {
        print_table_row(&app->cryptos[indexes[i]]);
    }
}

void crypto_print_all(const AppState *app)
{
    print_table_header();
    for (int i = 0; i < app->crypto_count; i++) {
        print_table_row(&app->cryptos[i]);
    }
}
