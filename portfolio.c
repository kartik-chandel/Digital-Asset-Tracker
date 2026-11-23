/* portfolio.c - holdings of the logged-in user */
#include <stdio.h>
#include "portfolio.h"
#include "calculator.h"
#include "crypto.h"
#include "market.h"
#include "transaction.h"
#include "user.h"
#include "utils.h"

int portfolio_find_holding(const AppState *app, int user_id, const char *symbol)
{
    for (int i = 0; i < app->holding_count; i++) {
        if (app->holdings[i].user_id == user_id &&
            str_equals_ignore_case(app->holdings[i].symbol, symbol)) {
            return i;
        }
    }
    return -1;
}

int portfolio_user_holding_count(const AppState *app, int user_id)
{
    int count = 0;

    for (int i = 0; i < app->holding_count; i++) {
        if (app->holdings[i].user_id == user_id) count++;
    }
    return count;
}

double portfolio_total_value(const AppState *app, int user_id)
{
    double total = 0.0;

    for (int i = 0; i < app->holding_count; i++) {
        const Holding *h = &app->holdings[i];
        double price;

        if (h->user_id != user_id) continue;
        price = market_get_price(app, h->symbol);
        if (price < 0.0) continue;                     /* unknown coin: skip */
        total += calc_current_value(h->quantity, price);
    }
    return total;
}

void portfolio_view(const AppState *app)
{
    int    uid   = app->current_user_id;
    double total = portfolio_total_value(app, uid);

    if (portfolio_user_holding_count(app, uid) == 0) {
        printf("\nYour portfolio is empty. Use \"Buy Cryptocurrency\" to add holdings.\n");
        return;
    }

    printf("\n%-7s %16s %16s %16s %18s %8s\n",
           "Symbol", "Quantity", "Avg Buy (INR)", "Price (INR)", "Value (INR)", "Alloc");
    printf("--------------------------------------------------------------"
           "------------------------\n");

    for (int i = 0; i < app->holding_count; i++) {
        const Holding *h = &app->holdings[i];
        double price, value, alloc;

        if (h->user_id != uid) continue;

        price = market_get_price(app, h->symbol);
        if (price < 0.0) price = 0.0;
        value = calc_current_value(h->quantity, price);
        alloc = (total > 0.0) ? (value / total) * 100.0 : 0.0;

        printf("%-7s %16.8f %16.2f %16.2f %18.2f %7.2f%%\n",
               h->symbol, h->quantity, h->avg_price, price, value, alloc);
    }

    printf("--------------------------------------------------------------"
           "------------------------\n");
    printf("Total portfolio value: %.2f INR\n", total);
}

void portfolio_remove_if_empty(AppState *app, int holding_index)
{
    if (holding_index < 0 || holding_index >= app->holding_count) return;
    if (app->holdings[holding_index].quantity > QTY_EPSILON) return;

    /* shift later holdings one place left */
    for (int i = holding_index; i < app->holding_count - 1; i++) {
        app->holdings[i] = app->holdings[i + 1];
    }
    app->holding_count--;
}

/* Menu option 2: record a coin the user ALREADY owns (opening position).
   It is stored as a BUY transaction so history and cost basis stay consistent. */
int portfolio_add_holding(AppState *app)
{
    int    ci;
    double quantity, price;

    if (!user_require_login(app)) return 0;

    printf("\n--- Add Existing Holding ---\n");
    printf("Use this for coins you already own. For new purchases use \"Buy\".\n");
    crypto_print_all(app);

    if (!crypto_prompt_symbol(app, "\nEnter coin symbol (blank to cancel): ", &ci)) return 0;

    if (portfolio_find_holding(app, app->current_user_id, app->cryptos[ci].symbol) >= 0) {
        printf("You already hold %s. Use \"Buy Cryptocurrency\" to add more.\n",
               app->cryptos[ci].symbol);
        return 0;
    }

    quantity = read_positive_double("Quantity you own: ", 0.0);
    price    = read_positive_double("Average price you paid per coin (INR): ", 0.0);

    printf("\nAdd %.8f %s at %.2f INR each (cost %.2f INR)?\n",
           quantity, app->cryptos[ci].symbol, price, quantity * price);
    if (!read_yes_no("Confirm (y/n): ")) {
        printf("Cancelled.\n");
        return 0;
    }

    if (!transaction_record_buy(app, app->cryptos[ci].symbol, quantity, price)) return 0;

    printf("Holding added. It was also recorded as a BUY transaction.\n");
    return 1;
}
