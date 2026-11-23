/* main.c - menus and program flow only. All real work lives in the modules. */
#include <stdio.h>
#include "common.h"
#include "crypto.h"
#include "market.h"
#include "portfolio.h"
#include "transaction.h"
#include "user.h"
#include "utils.h"

/* static = zero-initialised and kept off the stack (the struct is large) */
static AppState app;

static void not_implemented(const char *feature, int phase)
{
    printf("\n[%s] will be implemented in Phase %d.\n", feature, phase);
}

static void show_main_menu(void)
{
    printf("\n=============================\n");
    printf("     DIGITAL ASSET TRACKER\n");
    printf("=============================\n");
    printf("1. Register\n");
    printf("2. Login\n");
    printf("3. Exit\n");
}

static void show_dashboard_menu(void)
{
    printf("\n=============================\n");
    printf("        USER DASHBOARD\n");
    printf("  Logged in as: %s (ID %d)\n", user_current_name(&app), app.current_user_id);
    printf("=============================\n");
    printf("1.  View Portfolio\n");
    printf("2.  Add Cryptocurrency\n");
    printf("3.  Buy Cryptocurrency\n");
    printf("4.  Sell Cryptocurrency\n");
    printf("5.  View Market Prices\n");
    printf("6.  Calculate Profit/Loss\n");
    printf("7.  View Transaction History\n");
    printf("8.  Search Cryptocurrency\n");
    printf("9.  Generate Portfolio Report\n");
    printf("10. Logout\n");
}

static void handle_search(void)
{
    char keyword[NAME_LEN];
    int  matches[MAX_CRYPTOS];
    int  count;

    printf("Enter coin name or symbol: ");
    read_line(keyword, sizeof keyword);
    str_trim(keyword);

    if (keyword[0] == '\0') {
        printf("Search text cannot be empty.\n");
        return;
    }

    count = crypto_search(&app, keyword, matches, MAX_CRYPTOS);
    if (count == 0) {
        printf("No cryptocurrency found for \"%s\".\n", keyword);
    } else {
        crypto_print_matches(&app, matches, count);
    }
}

static void run_dashboard(void)
{
    int choice;

    do {
        show_dashboard_menu();
        choice = read_int_in_range("Enter choice (1-10): ", 1, 10);

        switch (choice) {
            case 1:  portfolio_view(&app);                          break;
            case 2:  portfolio_add_holding(&app);                   break;
            case 3:  transaction_buy(&app);                         break;
            case 4:  transaction_sell(&app);                        break;
            case 5:  market_print_prices(&app);                     break;
            case 6:  not_implemented("Calculate Profit/Loss", 3);   break;
            case 7:  transaction_print_history(&app);               break;
            case 8:  handle_search();                               break;
            case 9:  not_implemented("Portfolio Report", 4);        break;
            case 10: break;
        }

        if (choice != 10) {
            pause_for_enter();
        }
    } while (choice != 10);

    user_logout(&app);
    printf("\nLogged out.\n");
}

int main(void)
{
    int choice;

    app.next_user_id        = 1;
    app.next_transaction_id = 1;
    crypto_load_samples(&app);

    do {
        show_main_menu();
        choice = read_int_in_range("Enter choice (1-3): ", 1, 3);

        switch (choice) {
            case 1:
                user_register(&app);
                break;
            case 2:
                if (user_login(&app)) {
                    run_dashboard();
                }
                break;
            case 3:
                printf("\nNote: data is kept in memory only for now "
                       "(saving to files comes in Phase 4).\n");
                printf("Goodbye!\n");
                break;
        }
    } while (choice != 3);

    return 0;
}
