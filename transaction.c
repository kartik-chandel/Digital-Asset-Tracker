/* transaction.c - BUY / SELL flow and history */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "transaction.h"
#include "calculator.h"
#include "crypto.h"
#include "market.h"
#include "portfolio.h"
#include "user.h"
#include "utils.h"

void transaction_get_timestamp(char *buf, size_t size)
{
    time_t     now = time(NULL);
    struct tm *t   = localtime(&now);

    if (t == NULL || strftime(buf, size, "%Y-%m-%d %H:%M:%S", t) == 0) {
        snprintf(buf, size, "unknown");
    }
}

/* Adds one record to the history. Caller must check capacity first. */
static void append_transaction(AppState *app, const char *symbol, char type,
                               double quantity, double price)
{
    Transaction *t = &app->transactions[app->transaction_count];

    t->transaction_id = app->next_transaction_id;
    t->user_id        = app->current_user_id;
    snprintf(t->symbol, sizeof t->symbol, "%s", symbol);
    t->type           = type;
    t->quantity       = quantity;
    t->price          = price;
    t->total_value    = quantity * price;
    transaction_get_timestamp(t->date, sizeof t->date);

    app->transaction_count++;
    app->next_transaction_id++;
}

/* ---------- core logic ---------- */

int transaction_record_buy(AppState *app, const char *symbol,
                           double quantity, double price)
{
    int      uid = app->current_user_id;
    int      ci, hi;
    Holding *h;

    if (!user_require_login(app)) return 0;

    if (quantity <= 0.0 || price <= 0.0) {
        printf("Quantity and price must be greater than zero.\n");
        return 0;
    }

    ci = crypto_find_by_symbol(app, symbol);
    if (ci < 0) {
        printf("Unknown cryptocurrency \"%s\".\n", symbol);
        return 0;
    }

    if (app->transaction_count >= MAX_TRANSACTIONS) {
        printf("Transaction history is full. Cannot record more transactions.\n");
        return 0;
    }

    hi = portfolio_find_holding(app, uid, symbol);
    if (hi < 0) {                                   /* first purchase of this coin */
        if (app->holding_count >= MAX_HOLDINGS) {
            printf("Holdings storage is full.\n");
            return 0;
        }
        hi = app->holding_count++;
        memset(&app->holdings[hi], 0, sizeof app->holdings[hi]);
        app->holdings[hi].user_id = uid;
        snprintf(app->holdings[hi].symbol, sizeof app->holdings[hi].symbol,
                 "%s", app->cryptos[ci].symbol);
    }

    /* weighted-average cost: add to cost basis, then recompute the average */
    h = &app->holdings[hi];
    h->cost_basis += quantity * price;
    h->quantity   += quantity;
    h->avg_price   = calc_avg_price(h->cost_basis, h->quantity);

    append_transaction(app, h->symbol, TXN_BUY, quantity, price);
    return 1;
}

int transaction_record_sell(AppState *app, const char *symbol,
                            double quantity, double price, double *realized_out)
{
    int      uid = app->current_user_id;
    int      hi;
    Holding *h;
    double   realized;
    char     sym[SYMBOL_LEN];

    if (!user_require_login(app)) return 0;

    if (quantity <= 0.0 || price <= 0.0) {
        printf("Quantity and price must be greater than zero.\n");
        return 0;
    }

    hi = portfolio_find_holding(app, uid, symbol);
    if (hi < 0) {
        printf("You do not own any %s.\n", symbol);
        return 0;
    }
    h = &app->holdings[hi];

    /* never sell more than owned (tiny float differences are tolerated) */
    if (quantity > h->quantity + QTY_EPSILON) {
        printf("Insufficient holdings: you own %.8f %s.\n", h->quantity, h->symbol);
        return 0;
    }
    if (quantity > h->quantity) quantity = h->quantity;

    if (app->transaction_count >= MAX_TRANSACTIONS) {
        printf("Transaction history is full. Cannot record more transactions.\n");
        return 0;
    }

    /* realized P/L uses the average cost; cost basis shrinks proportionally */
    realized = calc_realized_pl_on_sell(quantity, price, h->avg_price);
    h->cost_basis -= quantity * h->avg_price;
    h->quantity   -= quantity;
    h->realized_pl += realized;
    if (h->cost_basis < 0.0) h->cost_basis = 0.0;

    snprintf(sym, sizeof sym, "%s", h->symbol);
    append_transaction(app, sym, TXN_SELL, quantity, price);

    /* NOTE: removing an empty holding also drops its realized_pl.
       Phase 3 will rebuild total realized P/L by replaying the history. */
    portfolio_remove_if_empty(app, hi);

    if (realized_out != NULL) *realized_out = realized;
    return 1;
}

/* ---------- interactive flows ---------- */

int transaction_buy(AppState *app)
{
    int           ci;
    double        quantity, price;
    const Crypto *c;

    if (!user_require_login(app)) return 0;

    printf("\n--- Buy Cryptocurrency ---\n");
    crypto_print_all(app);

    if (!crypto_prompt_symbol(app, "\nEnter symbol to buy (blank to cancel): ", &ci)) return 0;
    c = &app->cryptos[ci];

    printf("Current market price of %s: %.2f INR\n", c->symbol, c->price_inr);
    quantity = read_positive_double("Quantity to buy: ", 0.0);
    price    = read_positive_double("Buy price per coin in INR (Enter = market price): ",
                                    c->price_inr);

    printf("\nBUY %.8f %s at %.2f INR = %.2f INR total\n",
           quantity, c->symbol, price, quantity * price);
    if (!read_yes_no("Confirm purchase (y/n): ")) {
        printf("Cancelled.\n");
        return 0;
    }

    if (!transaction_record_buy(app, c->symbol, quantity, price)) return 0;

    printf("Purchase recorded.\n");
    return 1;
}

int transaction_sell(AppState *app)
{
    int            ci, hi;
    double         quantity, price, realized;
    const Crypto  *c;
    const Holding *h;

    if (!user_require_login(app)) return 0;

    printf("\n--- Sell Cryptocurrency ---\n");
    if (portfolio_user_holding_count(app, app->current_user_id) == 0) {
        printf("You have nothing to sell. Your portfolio is empty.\n");
        return 0;
    }
    portfolio_view(app);

    if (!crypto_prompt_symbol(app, "\nEnter symbol to sell (blank to cancel): ", &ci)) return 0;
    c = &app->cryptos[ci];

    hi = portfolio_find_holding(app, app->current_user_id, c->symbol);
    if (hi < 0) {
        printf("You do not own any %s.\n", c->symbol);
        return 0;
    }
    h = &app->holdings[hi];

    printf("You own %.8f %s (average buy price %.2f INR).\n",
           h->quantity, h->symbol, h->avg_price);
    printf("Current market price: %.2f INR\n", c->price_inr);

    quantity = read_positive_double("Quantity to sell (Enter = sell all): ", h->quantity);
    if (quantity > h->quantity + QTY_EPSILON) {
        printf("Insufficient holdings: you only own %.8f %s.\n", h->quantity, h->symbol);
        return 0;
    }
    price = read_positive_double("Sell price per coin in INR (Enter = market price): ",
                                 c->price_inr);

    printf("\nSELL %.8f %s at %.2f INR = %.2f INR total\n",
           quantity, c->symbol, price, quantity * price);
    printf("Expected realized profit/loss: %+.2f INR\n",
           calc_realized_pl_on_sell(quantity, price, h->avg_price));
    if (!read_yes_no("Confirm sale (y/n): ")) {
        printf("Cancelled.\n");
        return 0;
    }

    if (!transaction_record_sell(app, c->symbol, quantity, price, &realized)) return 0;

    printf("Sale recorded. Realized profit/loss: %+.2f INR\n", realized);
    return 1;
}

void transaction_print_history(const AppState *app)
{
    int uid   = app->current_user_id;
    int shown = 0;

    for (int i = 0; i < app->transaction_count; i++) {
        const Transaction *t = &app->transactions[i];

        if (t->user_id != uid) continue;

        if (shown == 0) {
            printf("\n%-5s %-19s %-5s %-7s %16s %16s %18s\n",
                   "ID", "Date", "Type", "Symbol", "Quantity", "Price (INR)", "Total (INR)");
            printf("----------------------------------------------------------"
                   "--------------------------------\n");
        }
        printf("%-5d %-19s %-5s %-7s %16.8f %16.2f %18.2f\n",
               t->transaction_id, t->date,
               t->type == TXN_BUY ? "BUY" : "SELL",
               t->symbol, t->quantity, t->price, t->total_value);
        shown++;
    }

    if (shown == 0) {
        printf("\nNo transactions yet.\n");
    }
}
